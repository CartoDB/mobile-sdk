#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "OnlineNMLModelLODTreeDataSource.h"
#include "core/BinaryData.h"
#include "graphics/ViewState.h"
#include "projections/Projection.h"
#include "renderers/components/CullState.h"
#include "utils/Log.h"
#include "utils/NetworkUtils.h"
#include "nml/nmlpackage/NMLPackage.pb.h"

#include <cglib/frustum3.h>

#include <boost/lexical_cast.hpp>

#define MINIZ_HEADER_FILE_ONLY
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include <miniz.c>

namespace carto {

    OnlineNMLModelLODTreeDataSource::OnlineNMLModelLODTreeDataSource(const std::shared_ptr<Projection>& projection, const std::string& serviceUrl) :
        NMLModelLODTreeDataSource(projection),
        _serviceUrl(serviceUrl)
    {
    }
    
    OnlineNMLModelLODTreeDataSource::~OnlineNMLModelLODTreeDataSource() {
    }
    
    std::vector<NMLModelLODTreeDataSource::MapTile> OnlineNMLModelLODTreeDataSource::loadMapTiles(const std::shared_ptr<CullState>& cullState) {
        MapBounds bounds(_projection->fromInternal(cullState->getEnvelope().getBounds().getMin()), _projection->fromInternal(cullState->getEnvelope().getBounds().getMax()));
    
        std::map<std::string, std::string> urlParams;
        urlParams["q"] = "MapTiles";
        urlParams["mapbounds_x0"] = boost::lexical_cast<std::string>(bounds.getMin().getX());
        urlParams["mapbounds_y0"] = boost::lexical_cast<std::string>(bounds.getMin().getY());
        urlParams["mapbounds_x1"] = boost::lexical_cast<std::string>(bounds.getMax().getX());
        urlParams["mapbounds_y1"] = boost::lexical_cast<std::string>(bounds.getMax().getY());
        urlParams["width"] = boost::lexical_cast<std::string>(_projection->getBounds().getDelta().getX());
        std::string url = NetworkUtils::BuildURLFromParameters(_serviceUrl, urlParams);
    
        Log::Debugf("OnlineNMLModelLODTreeDataSource: Request %s", url.c_str());
        std::shared_ptr<BinaryData> response;
        if (!NetworkUtils::GetHTTP(url, response, false)) {
            Log::Error("OnlineNMLModelLODTreeDataSource: Failed to receive tile list.");
            return std::vector<MapTile>();
        }
    
        DataInputStream gzipStream(*response->getDataPtr());
        int gzipDataSize = gzipStream.readInt();
        std::vector<unsigned char> gzipData = gzipStream.readBytes(gzipDataSize);
        std::vector<unsigned char> data;
        if (!inflate(gzipData, data)) {
            Log::Error("OnlineNMLModelLODTreeDataSource: Failed to decompress tile list data.");
            return std::vector<MapTile>();
        }
    
        DataInputStream dataStream(data);
        
        std::vector<MapTile> mapTiles;
        
        while (true) {
            long long mapTileId = dataStream.readLongLong();
            if (mapTileId == -1) {
                break;
            }
            long long modelLODTreeId = dataStream.readLongLong();
            double mapPosX = dataStream.readDouble();
            double mapPosY = dataStream.readDouble();
            double mapPosZ = dataStream.readDouble();
    
            MapTile mapTile(mapTileId, MapPos(mapPosX, mapPosY, mapPosZ), modelLODTreeId);
            mapTiles.push_back(mapTile);
        }
        return mapTiles;
    }
    
    std::shared_ptr<NMLModelLODTree> OnlineNMLModelLODTreeDataSource::loadModelLODTree(const MapTile& mapTile) {
        std::map<std::string, std::string> urlParams;
        urlParams["q"] = "ModelLODTree";
        urlParams["id"] = boost::lexical_cast<std::string>(mapTile.modelLODTreeId);
        std::string url = NetworkUtils::BuildURLFromParameters(_serviceUrl, urlParams);
    
        Log::Debugf("OnlineNMLModelLODTreeDataSource: Request %s", url.c_str());
        std::shared_ptr<BinaryData> response;
        if (!NetworkUtils::GetHTTP(url, response, false)) {
            Log::Error("OnlineNMLModelLODTreeDataSource: Failed to receive LOD tree.");
            return std::shared_ptr<NMLModelLODTree>();
        }
    
        DataInputStream gzipStream(*response->getDataPtr());
        int gzipDataSize = gzipStream.readInt();
        std::vector<unsigned char> gzipData = gzipStream.readBytes(gzipDataSize);
        std::vector<unsigned char> data;
        if (!inflate(gzipData, data)) {
            Log::Error("OnlineNMLModelLODTreeDataSource: Failed to decompress LOD tree.");
            return std::shared_ptr<NMLModelLODTree>();
        }
    
        DataInputStream dataStream(data);
        int nmlModelLODTreeSize = dataStream.readInt();
        std::vector<unsigned char> nmlModelLODTreeData = dataStream.readBytes(nmlModelLODTreeSize);
        std::shared_ptr<nml::ModelLODTree> sourceModelLODTree = std::make_shared<nml::ModelLODTree>(protobuf::message(nmlModelLODTreeData.size() > 0 ? &nmlModelLODTreeData[0] : nullptr, nmlModelLODTreeSize));
    
        // Model info, proxies
        NMLModelLODTree::ProxyMap proxyMap;
        while (true) {
            int modelId = dataStream.readInt();
            if (modelId == -1) {
                break;
            }
            std::string metaDataEnc = dataStream.readString();
            std::multimap<std::string, std::string> metaData = NetworkUtils::URLDecodeMap(metaDataEnc);
            double mapPosX = dataStream.readDouble();
            double mapPosY = dataStream.readDouble();
            double mapPosZ = dataStream.readDouble();
    
            std::shared_ptr<NMLModelLODTreeProxy> proxy(std::make_shared<NMLModelLODTreeProxy>(MapPos(mapPosX, mapPosY, mapPosZ)));
            proxy->setId(modelId);
            proxy->setMetaData(std::map<std::string, std::string>(metaData.begin(), metaData.end()));
            proxyMap[modelId] = proxy;
        }
    
        // Mesh bindings
        NMLModelLODTree::MeshBindingsMap meshBindingsMap;
        while (true) {
            int nodeId = dataStream.readInt();
            if (nodeId == -1) {
                break;
            }
            std::string localId = dataStream.readString();
            long long meshId = dataStream.readLongLong();
            if (meshId == -1) {
                meshId = dataStream.readLongLong();
                int nmlMeshOpSize = dataStream.readInt();
                std::vector<unsigned char> nmlMeshOpData = dataStream.readBytes(nmlMeshOpSize);
                std::shared_ptr<nml::MeshOp> meshOp = std::make_shared<nml::MeshOp>(protobuf::message(nmlMeshOpData.size() > 0 ? &nmlMeshOpData[0] : nullptr, nmlMeshOpSize));
                meshBindingsMap[nodeId].push_back(NMLModelLODTree::MeshBinding(meshId, localId, meshOp));
            } else {
                meshBindingsMap[nodeId].push_back(NMLModelLODTree::MeshBinding(meshId, localId));
            }
        }
    
        // Texture bindings
        NMLModelLODTree::TextureBindingsMap textureBindingsMap;
        while (true) {
            int nodeId = dataStream.readInt();
            if (nodeId == -1) {
                break;
            }
            std::string localId = dataStream.readString();
            long long textureId = dataStream.readLongLong();
            int level = dataStream.readInt();
    
            textureBindingsMap[nodeId].push_back(NMLModelLODTree::TextureBinding(textureId, level, localId));
        }
    
        std::shared_ptr<NMLModelLODTree> modelLODTree = std::make_shared<NMLModelLODTree>(mapTile.modelLODTreeId, mapTile.mapPos, _projection, sourceModelLODTree, proxyMap, meshBindingsMap, textureBindingsMap);
        return modelLODTree;
    }
    
    std::shared_ptr<nml::Mesh> OnlineNMLModelLODTreeDataSource::loadMesh(long long meshId) {
        std::map<std::string, std::string> urlParams;
        urlParams["q"] = "Meshes";
        urlParams["ids"] = boost::lexical_cast<std::string>(meshId);
        std::string url = NetworkUtils::BuildURLFromParameters(_serviceUrl, urlParams);
    
        Log::Debugf("OnlineNMLModelLODTreeDataSource: Request %s", url.c_str());
        std::shared_ptr<BinaryData> response;
        if (!NetworkUtils::GetHTTP(url, response, false)) {
            Log::Error("OnlineNMLModelLODTreeDataSource: Failed to receive mesh data.");
            return std::shared_ptr<nml::Mesh>();
        }
    
        DataInputStream gzipStream(*response->getDataPtr());
        gzipStream.readLongLong();
        int gzipDataSize = gzipStream.readInt();
        std::vector<unsigned char> gzipData = gzipStream.readBytes(gzipDataSize);
        std::vector<unsigned char> data;
        if (!inflate(gzipData, data)) {
            Log::Error("OnlineNMLModelLODTreeDataSource: Failed to decompress mesh data.");
            return std::shared_ptr<nml::Mesh>();
        }
    
        std::shared_ptr<nml::Mesh> mesh = std::make_shared<nml::Mesh>(protobuf::message(data.size() > 0 ? &data[0] : nullptr, data.size()));
        return mesh;
    }
    
    std::shared_ptr<nml::Texture> OnlineNMLModelLODTreeDataSource::loadTexture(long long textureId, int level) {
        std::map<std::string, std::string> urlParams;
        urlParams["q"] = "Textures";
        urlParams["ids"] = boost::lexical_cast<std::string>(textureId);
        std::string url = NetworkUtils::BuildURLFromParameters(_serviceUrl, urlParams);
    
        Log::Debugf("OnlineNMLModelLODTreeDataSource: Request %s", url.c_str());
        std::shared_ptr<BinaryData> response;
        if (!NetworkUtils::GetHTTP(url, response, false)) {
            Log::Error("OnlineNMLModelLODTreeDataSource: Failed to receive texture data.");
            return std::shared_ptr<nml::Texture>();
        }
    
        DataInputStream gzipStream(*response->getDataPtr());
        gzipStream.readLongLong();
        int gzipDataSize = gzipStream.readInt();
        std::vector<unsigned char> gzipData = gzipStream.readBytes(gzipDataSize);
        std::vector<unsigned char> data;
        if (!inflate(gzipData, data)) {
            Log::Error("OnlineNMLModelLODTreeDataSource: Failed to decompress texture data.");
            return std::shared_ptr<nml::Texture>();
        }
    
        std::shared_ptr<nml::Texture> texture = std::make_shared<nml::Texture>(protobuf::message(data.size() > 0 ? &data[0] : nullptr, data.size()));
        return texture;
    }
    
    bool OnlineNMLModelLODTreeDataSource::inflate(const std::vector<unsigned char>& in, std::vector<unsigned char>& out) {
        if (in.size() < 14) {
            return false;
        }
    
        std::size_t offset = 0;
        if (in[0] != 0x1f || in[1] != 0x8b) {
            return false;
        }
        if (in[2] != 8) {
            return false;
        }
        int flags = in[3];
        offset += 10;
        if (flags & (1 << 2)) { // FEXTRA
            int n = static_cast<int>(in[offset + 0]) | (static_cast<int>(in[offset + 1]) << 8);
            offset += n + 2;
        }
        if (flags & (1 << 3)) { // FNAME
            while (offset < in.size()) {
                if (in[offset++] == 0) {
                    break;
                }
            }
        }
        if (flags & (1 << 4)) { // FCOMMENT
            while (offset < in.size()) {
                if (in[offset++] == 0) {
                    break;
                }
            }
        }
        if (flags & (1 << 1)) { // FCRC
            offset += 2;
        }
    
        unsigned char buf[4096];
        ::mz_stream infstream;
        std::memset(&infstream, 0, sizeof(infstream));
        infstream.zalloc = NULL;
        infstream.zfree = NULL;
        infstream.opaque = NULL;
        int err = MZ_OK;
        infstream.avail_in = static_cast<unsigned int>(in.size() - offset - 4); // size of input
        infstream.next_in = reinterpret_cast<const unsigned char *>(&in[offset]); // input char array
        infstream.avail_out = sizeof(buf); // size of output
        infstream.next_out = buf; // output char array
        ::mz_inflateInit2(&infstream, -MZ_DEFAULT_WINDOW_BITS);
        do {
            infstream.avail_out = sizeof(buf); // size of output
            infstream.next_out = buf; // output char array
            err = ::mz_inflate(&infstream, infstream.avail_in > 0 ? MZ_NO_FLUSH : MZ_FINISH);
            if (err != MZ_OK && err != MZ_STREAM_END) {
                break;
            }
            out.insert(out.end(), buf, buf + sizeof(buf) - infstream.avail_out);
        } while (err != MZ_STREAM_END);
        ::mz_inflateEnd(&infstream);
        return err == MZ_OK || err == MZ_STREAM_END;
    }
    
    OnlineNMLModelLODTreeDataSource::DataInputStream::DataInputStream(const std::vector<unsigned char>& data) : _data(data), _offset(0)
    {
    }
    
    unsigned char OnlineNMLModelLODTreeDataSource::DataInputStream::readByte() {
        if (_offset >= _data.size()) {
            Log::Error("OnlineNMLModelLODTreeDataSource::DataInputStream: reading past the end");
            return 0;
        }
        return _data[_offset++];
    }
    
    int OnlineNMLModelLODTreeDataSource::DataInputStream::readInt() {
        int value = 0;
        for (int i = 0; i < 4; i++) {
            value = (value << 8) | readByte();
        }
        return value;
    }
    
    long long OnlineNMLModelLODTreeDataSource::DataInputStream::readLongLong() {
        long long value = 0;
        for (int i = 0; i < 8; i++) {
            value = (value << 8) | readByte();
        }
        return value;
    }
    
    float OnlineNMLModelLODTreeDataSource::DataInputStream::readFloat() {
        int value = readInt();
        return *reinterpret_cast<float *>(&value);
    }
    
    double OnlineNMLModelLODTreeDataSource::DataInputStream::readDouble() {
        long long value = readLongLong();
        return *reinterpret_cast<double *>(&value);
    }
    
    std::string OnlineNMLModelLODTreeDataSource::DataInputStream::readString() {
        unsigned int length = readByte();
        length = (length << 8) | readByte();
        if (_offset + length > _data.size()) {
            Log::Error("OnlineNMLModelLODTreeDataSource::DataInputStream: reading past the end");
            return std::string();
        }
        std::size_t offset = _offset;
        _offset += length;
        return std::string(reinterpret_cast<const char *>(_data.data() + offset), reinterpret_cast<const char *>(_data.data() + _offset));
    }
    
    std::vector<unsigned char> OnlineNMLModelLODTreeDataSource::DataInputStream::readBytes(size_t size) {
        if (_offset + size > _data.size()) {
            Log::Error("OnlineNMLModelLODTreeDataSource::DataInputStream: reading past the end");
            return std::vector<unsigned char>();
        }
        std::size_t offset = _offset;
        _offset += size;
        return std::vector<unsigned char>(_data.begin() + offset, _data.begin() + _offset);
    }
    
}

#endif
