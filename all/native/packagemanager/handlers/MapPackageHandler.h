/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPPACKAGEHANDLER_H_
#define _CARTO_MAPPACKAGEHANDLER_H_

#if defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "core/MapTile.h"
#include "packagemanager/handlers/PackageHandler.h"

#include <vector>

namespace sqlite3pp {
    class database;
    namespace ext {
        class function;
    }
}

namespace carto {
    class BinaryData;
    
    class MapPackageHandler : public PackageHandler {
    public:
        MapPackageHandler(const std::string& fileName, const std::string& serverEncKey, const std::string& localEncKey);
        virtual ~MapPackageHandler();

        void openDatabase();
        void closeDatabase();
        std::shared_ptr<BinaryData> loadTile(const MapTile& mapTile);

        virtual void onImportPackage();
        virtual void onDeletePackage();

        virtual std::shared_ptr<PackageTileMask> calculateTileMask() const;

    private:
        static bool CheckDbEncryption(sqlite3pp::database& db, const std::string& encKey);
        static void UpdateDbEncryption(sqlite3pp::database& db, const std::string& encKey);

        static std::string CalculateKeyHash(const std::string& encKey);
        static void EncryptTile(std::vector<unsigned char>& data, int zoom, int x, int y, const std::string& encKey);
        static void DecryptTile(std::vector<unsigned char>& data, int zoom, int x, int y, const std::string& encKey);
        static void SetCipherKeyIV(unsigned char* k, unsigned char* iv, int zoom, int x, int y, const std::string& encKey);

        const std::string _serverEncKey;
        const std::string _localEncKey;

        std::unique_ptr<sqlite3pp::database> _packageDb;
        std::unique_ptr<sqlite3pp::ext::function> _decryptFunc;
        std::unique_ptr<BinaryData> _sharedDictionary;
    };
    
}

#endif

#endif
