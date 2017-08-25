#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "PackageTileMask.h"

#include <vector>
#include <algorithm>

namespace {
    enum { NP = 255 };

    static const unsigned char base64DecodeTable[] = {
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,62,NP,62,NP,63,52,53,
        54,55,56,57,58,59,60,61,NP,NP,
        NP,NP,NP,NP,NP, 0, 1, 2, 3, 4,
         5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,
        25,NP,NP,NP,NP,63,NP,26,27,28,
        29,30,31,32,33,34,35,36,37,38,
        39,40,41,42,43,44,45,46,47,48,
        49,50,51,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP,NP,NP,NP,NP,
        NP,NP,NP,NP,NP,NP
    };

    static const char base64EncodeTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
        'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
        'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '+', '/'
    };

    std::queue<bool> decodeBase64(const std::string& stringValue) {
        std::queue<bool> data;
        for (char c : stringValue) {
            int val = base64DecodeTable[static_cast<unsigned char>(c)];
            for (int i = 5; i >= 0; i--) {
                data.push(((val >> i) & 1) != 0);
            }
        }
        return data;
    }

    std::string encodeBase64(std::vector<bool> data) {
        while (data.size() % 24 != 0) {
            data.push_back(false);
        }
        std::string stringValue;
        stringValue.reserve(data.size() / 6);
        unsigned char val = 0;
        for (std::size_t i = 0; i < data.size(); i++) {
            val = (val << 1) | (data[i] ? 1 : 0);
            if ((i + 1) % 6 == 0) {
                stringValue.push_back(base64EncodeTable[val]);
                val = 0;
            }
        }
        return stringValue;
    }
}

namespace carto {
    
    PackageTileMask::PackageTileMask(const std::string& stringValue, int maxZoom) :
        _stringValue(stringValue),
        _rootNode(),
        _maxZoomLevel(maxZoom)
    {
        std::queue<bool> data = decodeBase64(stringValue);
        _rootNode = decodeTileNode(data, MapTile(0, 0, 0, 0));
    }

    PackageTileMask::PackageTileMask(const std::vector<MapTile>& tiles, int clipZoom) :
        _stringValue(),
        _rootNode(),
        _maxZoomLevel(0)
    {
        std::unordered_set<MapTile> tileSet(tiles.begin(), tiles.end());
        _rootNode = buildTileNode(tileSet, MapTile(0, 0, 0, 0), clipZoom);
        for (const MapTile& tile : tiles) {
            _maxZoomLevel = std::max(_maxZoomLevel, tile.getZoom());
        }
        std::vector<bool> data = encodeTileNode(_rootNode);
        _stringValue = encodeBase64(std::move(data));
    }

    const std::string& PackageTileMask::getStringValue() const {
        return _stringValue;
    }
    
    std::string PackageTileMask::getURLSafeStringValue() const {
        std::string val = _stringValue;
        std::replace(val.begin(), val.end(), '+', '-');
        std::replace(val.begin(), val.end(), '/', '_');
        return val;
    }
    
    int PackageTileMask::getMaxZoomLevel() const {
        return _maxZoomLevel;
    }

    PackageTileStatus::PackageTileStatus PackageTileMask::getTileStatus(const MapTile& mapTile) const {
        if (std::shared_ptr<TileNode> node = findTileNode(mapTile)) {
            if (mapTile.getZoom() <= _maxZoomLevel) {
                return (node->inside ? PackageTileStatus::PACKAGE_TILE_STATUS_FULL : PackageTileStatus::PACKAGE_TILE_STATUS_MISSING);
            }
        }
        return PackageTileStatus::PACKAGE_TILE_STATUS_MISSING;
    }

    std::shared_ptr<PackageTileMask::TileNode> PackageTileMask::findTileNode(const MapTile& tile) const {
        if (tile.getZoom() == 0) {
            if (tile.getX() == 0 && tile.getY() == 0) {
                return _rootNode;
            }
            return std::shared_ptr<TileNode>();
        }

        std::shared_ptr<TileNode> parentNode = findTileNode(MapTile(tile.getX() / 2, tile.getY() / 2, tile.getZoom() - 1, tile.getFrameNr()));
        if (parentNode) {
            for (int i = 0; i < 4; i++) {
                const std::shared_ptr<TileNode>& node = parentNode->subNodes[i];
                if (node) {
                    if (node->tile == tile) {
                        return node;
                    }
                }
            }
            if (parentNode->inside) {
                return parentNode;
            }
        }
        return std::shared_ptr<TileNode>();
    }

    std::shared_ptr<PackageTileMask::TileNode> PackageTileMask::buildTileNode(const std::unordered_set<MapTile>& tileSet, const MapTile& tile, int clipZoom) {
        auto node = std::make_shared<TileNode>(tile, tileSet.find(tile) != tileSet.end());
        if (!node->inside || tile.getZoom() >= clipZoom) {
            return node; // Note: we assume here that tile does not exist implies subtiles do not exist
        }

        int idx = 0;
        bool deep = false;
        for (int dy = 0; dy < 2; dy++) {
            for (int dx = 0; dx < 2; dx++) {
                node->subNodes[idx] = buildTileNode(tileSet, MapTile(tile.getX() * 2 + dx, tile.getY() * 2 + dy, tile.getZoom() + 1, tile.getFrameNr()), clipZoom);
                for (int i = 0; i < 4; i++) {
                    deep = deep || node->subNodes[idx]->subNodes[i];
                }
                idx++;
            }
        }
        if (!deep) {
            if (node->subNodes[0]->inside && node->subNodes[1]->inside && node->subNodes[2]->inside && node->subNodes[3]->inside) {
                node->subNodes[0] = node->subNodes[1] = node->subNodes[2] = node->subNodes[3] = std::shared_ptr<TileNode>();
            }
        }
        return node;
    }
    
    std::shared_ptr<PackageTileMask::TileNode> PackageTileMask::decodeTileNode(std::queue<bool>& data, const MapTile& tile) {
        bool leaf = !data.front();
        data.pop();
        bool inside = data.front();
        data.pop();
        auto node = std::make_shared<TileNode>(tile, inside);
        if (!leaf) {
            int idx = 0;
            for (int dy = 0; dy < 2; dy++) {
                for (int dx = 0; dx < 2; dx++) {
                    node->subNodes[idx] = decodeTileNode(data, MapTile(tile.getX() * 2 + dx, tile.getY() * 2 + dy, tile.getZoom() + 1, tile.getFrameNr()));
                    idx++;
                }
            }
        }
        return node;
    }

    std::vector<bool> PackageTileMask::encodeTileNode(const std::shared_ptr<TileNode>& node) {
        std::vector<bool> data;
        if (!node) {
            return data;
        }
        for (int idx = 0; idx < 4; idx++) {
            std::vector<bool> subData = encodeTileNode(node->subNodes[idx]);
            data.insert(data.end(), subData.begin(), subData.end());
        }
        if (data.empty()) {
            data.push_back(false);
            data.push_back(node->inside);
        }
        else {
            data.insert(data.begin(), true);
            data.insert(data.begin() + 1, node->inside);
        }
        return data;
    }

}

#endif