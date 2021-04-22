#if defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "MapPackageHandler.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "packagemanager/PackageTileMask.h"
#include "utils/Log.h"

#include <stdext/zlib.h>

#include <sqlite3pp.h>
#include <sqlite3ppext.h>

#include <cryptopp/rc5.h>
#include <cryptopp/sha.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

namespace carto {

    MapPackageHandler::MapPackageHandler(const std::string& fileName, const std::string& serverEncKey, const std::string& localEncKey) :
        PackageHandler(fileName),
        _serverEncKey(serverEncKey),
        _localEncKey(localEncKey),
        _packageDb(),
        _decryptFunc(),
        _sharedDictionary()
    {
    }

    MapPackageHandler::~MapPackageHandler() {
    }

    void MapPackageHandler::openDatabase() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        if (_packageDb) {
            return;
        }

        try {
            // Open package database
            _packageDb.reset(new sqlite3pp::database());
            if (_packageDb->connect_v2(_fileName.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) {
                Log::Errorf("MapPackageHandler::openDatabase: Failed to open database %s", _fileName.c_str());
                return;
            }

            // Create new sqlite decryption function. First check if the database is crypted.
            std::string encKey = _serverEncKey;
            bool encrypted = CheckDbEncryption(*_packageDb, _serverEncKey + _localEncKey); // NOTE: this is a hack - though tiles are actually encrypted with server key only, with check that local key is included in the hash also
            _decryptFunc.reset(new sqlite3pp::ext::function(*_packageDb));
            _decryptFunc->create("tile_decrypt", [encrypted, encKey](sqlite3pp::ext::context& ctx) {
                const unsigned char* encData = reinterpret_cast<const unsigned char*>(ctx.get<const void*>(0));
                std::size_t encSize = ctx.args_bytes(0);
                int zoom = ctx.get<int>(1);
                int x = ctx.get<int>(2);
                int y = ctx.get<int>(3);
                std::vector<unsigned char> encVector(encData, encData + encSize);
                if (encrypted) {
                    DecryptTile(encVector, zoom, x, y, encKey);
                }
                ctx.result(encVector.empty() ? nullptr : &encVector[0], static_cast<int>(encVector.size()), false);
            }, 4);

            // Try to load shared dictionary
            _sharedDictionary.reset();
            sqlite3pp::query query(*_packageDb, "SELECT value FROM metadata WHERE name='shared_zlib_dict'");
            for (auto qit = query.begin(); qit != query.end(); qit++) {
                const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(qit->get<const void*>(0));
                std::size_t dataSize = qit->column_bytes(0);
                _sharedDictionary.reset(new BinaryData(dataPtr, dataSize));
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("MapPackageHandler::openDatabase: Exception %s", ex.what());
        }
    }

    void MapPackageHandler::closeDatabase() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        _packageDb.reset();
        _decryptFunc.reset();
        _sharedDictionary.reset();
    }

    std::shared_ptr<BinaryData> MapPackageHandler::loadTile(const MapTile& mapTile) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        try {
            openDatabase();

            // Try to load the tile (this could fail, as tile masks may not be complete to the last zoom level)
            sqlite3pp::query query(*_packageDb, "SELECT tile_decrypt(tile_data, zoom_level, tile_column, tile_row) FROM tiles WHERE zoom_level=:zoom AND tile_column=:x AND tile_row=:y");
            query.bind(":zoom", mapTile.getZoom());
            query.bind(":x", mapTile.getX());
            query.bind(":y", mapTile.getY());
            for (auto qit = query.begin(); qit != query.end(); qit++) {
                const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(qit->get<const void*>(0));
                std::size_t dataSize = qit->column_bytes(0);
                std::vector<unsigned char> data(dataPtr, dataPtr + dataSize);
                if (_sharedDictionary) {
                    std::vector<unsigned char> uncompressedData;
                    if (!zlib::inflate_raw(data.data(), data.size(), _sharedDictionary->data(), _sharedDictionary->size(), uncompressedData)) {
                        Log::Warnf("MapPackageHandler::loadTile: Failed to decompress tile with shared dictionary");
                        return std::shared_ptr<BinaryData>();
                    }
                    std::swap(data, uncompressedData);
                }
                return std::make_shared<BinaryData>(std::move(data));
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("MapPackageHandler::loadTile: Exception %s", ex.what());
        }
        return std::shared_ptr<BinaryData>();
    }

    void MapPackageHandler::onImportPackage() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        sqlite3pp::database packageDb;
        if (packageDb.connect_v2(_fileName.c_str(), SQLITE_OPEN_READWRITE) != SQLITE_OK) {
            Log::Errorf("MapPackageHandler::onImportPackage: Failed to open database %s", _fileName.c_str());
            return;
        }
        bool encrypted = CheckDbEncryption(packageDb, _serverEncKey);
        if (encrypted) {
            UpdateDbEncryption(packageDb, _serverEncKey + _localEncKey);
        }
    }

    void MapPackageHandler::onDeletePackage() {
    }

    std::shared_ptr<PackageTileMask> MapPackageHandler::calculateTileMask() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        // Fetch all tile coordinates and create tilemask from them
        sqlite3pp::database packageDb;
        if (packageDb.connect_v2(_fileName.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) {
            Log::Errorf("MapPackageHandler::calculateTileMask: Failed to open database %s", _fileName.c_str());
            return std::shared_ptr<PackageTileMask>();
        }
        sqlite3pp::query query(packageDb, "SELECT zoom_level, tile_column, tile_row FROM tiles");
        std::vector<MapTile> tiles;
        int maxZoomLevel = 0;
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            MapTile tile(qit->get<int>(1), qit->get<int>(2), qit->get<int>(0), 0);
            maxZoomLevel = std::max(maxZoomLevel, tile.getZoom());
            tiles.push_back(tile);
        }
        return std::make_shared<PackageTileMask>(tiles, maxZoomLevel);
    }

    bool MapPackageHandler::CheckDbEncryption(sqlite3pp::database& db, const std::string& encKey) {
        sqlite3pp::query query(db, "SELECT value FROM metadata WHERE name='nutikeysha1'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            if (encKey.empty()) {
                throw GenericException("Package database is encrypted and needs encryption key");
            }
            std::string sha1 = qit->get<const char*>(0);
            if (sha1 != CalculateKeyHash(encKey)) {
                throw GenericException("Package encryption keys do not match");
            }
            return true;
        }
        return false;
    }

    void MapPackageHandler::UpdateDbEncryption(sqlite3pp::database& db, const std::string& encKey) {
        sqlite3pp::transaction xct(db);
        {
            sqlite3pp::command command(db, "DELETE FROM metadata WHERE name='nutikeysha1'");
            command.execute();
            if (!encKey.empty()) {
                std::string sha1 = CalculateKeyHash(encKey);
                sqlite3pp::command command2(db, "INSERT INTO metadata(name, value) VALUES('nutikeysha1', :hash)");
                command2.bind(":hash", sha1.c_str());
                command2.execute();
            }
            xct.commit();
        }
    }
    
    std::string MapPackageHandler::CalculateKeyHash(const std::string& encKey) {
        CryptoPP::SHA1 hash;
        unsigned char digest[CryptoPP::SHA1::DIGESTSIZE];
        hash.CalculateDigest(digest, reinterpret_cast<const unsigned char*>(encKey.c_str()), encKey.size());
        std::string sha1;
        CryptoPP::HexEncoder encoder;
        encoder.Attach(new CryptoPP::StringSink(sha1));
        encoder.Put(digest, sizeof(digest));
        encoder.MessageEnd();
        return sha1;
    }

    void MapPackageHandler::EncryptTile(std::vector<unsigned char>& data, int zoom, int x, int y, const std::string& encKey) {
        if (data.empty()) {
            return;
        }
        
        unsigned char iv[CryptoPP::RC5::BLOCKSIZE];
        unsigned char k[CryptoPP::RC5::DEFAULT_KEYLENGTH];
        SetCipherKeyIV(k, iv, zoom, x, y, encKey);
        CryptoPP::CBC_Mode<CryptoPP::RC5>::Encryption enc;
        enc.SetKeyWithIV(k, sizeof(k), iv);
        std::string cipherText;
        cipherText.reserve(data.size() + 1);
        CryptoPP::StreamTransformationFilter stfEncryptor(enc, new CryptoPP::StringSink(cipherText), CryptoPP::StreamTransformationFilter::PKCS_PADDING); // NOTE: stfEncryptor will delete sink itself
        stfEncryptor.Put(&data[0], data.size());
        stfEncryptor.MessageEnd();
        data.assign(reinterpret_cast<const unsigned char*>(cipherText.data()), reinterpret_cast<const unsigned char*>(cipherText.data() + cipherText.size()));
    }
    
    void MapPackageHandler::DecryptTile(std::vector<unsigned char>& data, int zoom, int x, int y, const std::string& encKey) {
        if (data.empty()) {
            return;
        }
        
        unsigned char iv[CryptoPP::RC5::BLOCKSIZE];
        unsigned char k[CryptoPP::RC5::DEFAULT_KEYLENGTH];
        SetCipherKeyIV(k, iv, zoom, x, y, encKey);
        CryptoPP::CBC_Mode<CryptoPP::RC5>::Decryption dec;
        dec.SetKeyWithIV(k, sizeof(k), iv);
        std::string plainText;
        plainText.reserve(data.size() + 1);
        CryptoPP::StreamTransformationFilter stfEncryptor(dec, new CryptoPP::StringSink(plainText), CryptoPP::StreamTransformationFilter::PKCS_PADDING); // NOTE: stfEncryptor will delete sink itself
        stfEncryptor.Put(&data[0], data.size());
        stfEncryptor.MessageEnd();
        data.assign(reinterpret_cast<const unsigned char*>(plainText.data()), reinterpret_cast<const unsigned char*>(plainText.data() + plainText.size()));
    }

    void MapPackageHandler::SetCipherKeyIV(unsigned char* k, unsigned char* iv, int zoom, int x, int y, const std::string& encKey) {
        std::fill(iv, iv + CryptoPP::RC5::BLOCKSIZE, 0);
        iv[0 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>(zoom);
        iv[1 % CryptoPP::RC5::BLOCKSIZE] ^= 0;
        iv[2 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((x >> 0)  & 255);
        iv[3 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((x >> 8)  & 255);
        iv[4 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((x >> 16) & 255);
        iv[5 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((y >> 0)  & 255);
        iv[6 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((y >> 8)  & 255);
        iv[7 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((y >> 16) & 255);
        std::fill(k, k + CryptoPP::RC5::DEFAULT_KEYLENGTH, 0);
        std::copy(encKey.begin(), encKey.begin() + std::min(encKey.size(), static_cast<std::size_t>(CryptoPP::RC5::DEFAULT_KEYLENGTH)), k);
    }

}

#endif
