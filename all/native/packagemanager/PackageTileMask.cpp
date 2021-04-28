#include "PackageTileMask.h"
#include "core/MapBounds.h"
#include "components/Exceptions.h"
#include "geometry/MultiPolygonGeometry.h"
#include "projections/Projection.h"
#include "utils/TileUtils.h"

#include <vector>
#include <algorithm>

namespace {
    enum { NP = 255 };

    const unsigned char base64DecodeTable[] = {
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

    std::vector<bool> decodeBase64(const std::string& stringValue) {
        std::vector<bool> data;
        data.reserve(stringValue.size() * 6);
        for (char c : stringValue) {
            int val = base64DecodeTable[static_cast<unsigned char>(c)];
            for (int i = 5; i >= 0; i--) {
                data.push_back(((val >> i) & 1) != 0);
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

    std::vector<std::vector<carto::MapPos> > createTilePolygon(const carto::MapTile& mapTile, const std::shared_ptr<carto::Projection>& proj) {
        std::vector<carto::MapPos> poses;
        carto::MapBounds bounds = carto::TileUtils::CalculateMapTileBounds(mapTile, proj);
        poses.emplace_back(bounds.getMin().getX(), bounds.getMin().getY());
        poses.emplace_back(bounds.getMax().getX(), bounds.getMin().getY());
        poses.emplace_back(bounds.getMax().getX(), bounds.getMax().getY());
        poses.emplace_back(bounds.getMin().getX(), bounds.getMax().getY());
        return std::vector<std::vector<carto::MapPos> > {{ poses }};
    }

    std::vector<std::vector<carto::MapPos> > unifyTilePolygons(const std::vector<std::vector<carto::MapPos> >& poly1, const std::vector<std::vector<carto::MapPos> >& poly2) {
        std::unordered_set<std::size_t> poly2Indices;
        std::vector<std::vector<carto::MapPos> > unifiedPoly;
        for (const std::vector<carto::MapPos>& poses1 : poly1) {
            bool found = false;
            for (std::size_t i = 0; i < poses1.size() && !found; i++) {
                for (auto it2 = poly2.begin(); it2 != poly2.end() && !found; it2++) {
                    if (poly2Indices.find(it2 - poly2.begin()) != poly2Indices.end()) {
                        continue;
                    }

                    const std::vector<carto::MapPos>& poses2 = *it2;
                    for (std::size_t j = 0; j < poses2.size(); j++) {
                        if (poses1[i] != poses2[j]) {
                            continue;
                        }

                        std::size_t i0 = (i + poses1.size() - 1) % poses1.size();
                        std::size_t j1 = (j + 1) % poses2.size();
                        if ((poses1[i] - poses1[i0]).crossProduct2D(poses2[j1] - poses2[j]) > 0) {
                            continue;
                        }

                        std::vector<carto::MapPos> unifiedPoses;
                        unifiedPoses.reserve(poses1.size() + poses2.size());
                        unifiedPoses.insert(unifiedPoses.end(), poses1.begin(), poses1.begin() + i);
                        unifiedPoses.insert(unifiedPoses.end(), poses2.begin() + j, poses2.end());
                        unifiedPoses.insert(unifiedPoses.end(), poses2.begin(), poses2.begin() + j);
                        unifiedPoses.insert(unifiedPoses.end(), poses1.begin() + i, poses1.end());

                        for (std::size_t k = 1; k < unifiedPoses.size(); ) {
                            carto::MapVec v0 = unifiedPoses[k] - unifiedPoses[k - 1];
                            carto::MapVec v1 = unifiedPoses[k] - unifiedPoses[(k + 1) % unifiedPoses.size()];
                            if (v0 == v1) {
                                unifiedPoses.erase(unifiedPoses.begin() + k - 1, unifiedPoses.begin() + k + 1);
                            } else {
                                k++;
                            }
                        }
                        unifiedPoly.push_back(std::move(unifiedPoses));
                        poly2Indices.insert(it2 - poly2.begin());
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                unifiedPoly.push_back(poses1);
            }
        }
        for (auto it2 = poly2.begin(); it2 != poly2.end(); it2++) {
            if (poly2Indices.find(it2 - poly2.begin()) == poly2Indices.end()) {
                unifiedPoly.push_back(*it2);
            }
        }
        return unifiedPoly;
    }

}

namespace carto {
    
    PackageTileMask::PackageTileMask(const std::string& stringValue, int maxZoom) :
        _stringValue(stringValue),
        _maxZoomLevel(maxZoom),
        _cachedRootNode(),
        _mutex()
    {
    }

    PackageTileMask::PackageTileMask(const std::vector<MapTile>& tiles, int clipZoom) :
        _stringValue(),
        _maxZoomLevel(0),
        _cachedRootNode(),
        _mutex()
    {
        std::unordered_set<MapTile> tileSet(tiles.begin(), tiles.end());
        _cachedRootNode = std::make_unique<TileNode>();
        BuildTileNode(*_cachedRootNode, tileSet, MapTile(0, 0, 0, 0), clipZoom);
        for (const MapTile& tile : tiles) {
            _maxZoomLevel = std::max(_maxZoomLevel, tile.getZoom());
        }
        std::vector<bool> data;
        EncodeTileNode(*_cachedRootNode, data);
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

    std::shared_ptr<MultiPolygonGeometry> PackageTileMask::getBoundingPolygon(const std::shared_ptr<Projection>& projection) const {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }

        std::vector<std::vector<MapPos> > poly = CalculateTileNodeBoundingPolygon(*getRootNode(), projection);

        std::vector<std::vector<MapPos> > optimizedPoly;
        for (std::size_t i = 0; i < poly.size(); i++) {
            optimizedPoly = unifyTilePolygons(optimizedPoly, std::vector<std::vector<MapPos> >(poly.begin() + i, poly.begin() + i + 1));
        }

        std::vector<std::shared_ptr<PolygonGeometry> > geoms;
        geoms.push_back(std::make_shared<PolygonGeometry>(std::move(optimizedPoly)));
        return std::make_shared<MultiPolygonGeometry>(geoms);
    }

    PackageTileStatus::PackageTileStatus PackageTileMask::getTileStatus(const MapTile& mapTile) const {
        if (const TileNode* node = findTileNode(mapTile)) {
            if (mapTile.getZoom() <= _maxZoomLevel) {
                return (node->inside ? PackageTileStatus::PACKAGE_TILE_STATUS_FULL : PackageTileStatus::PACKAGE_TILE_STATUS_MISSING);
            }
        }
        return PackageTileStatus::PACKAGE_TILE_STATUS_MISSING;
    }

    const PackageTileMask::TileNode* PackageTileMask::getRootNode() const {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_cachedRootNode) {
            _cachedRootNode = std::make_unique<TileNode>();
            std::size_t offset = 0;
            DecodeTileNode(*_cachedRootNode, decodeBase64(_stringValue), offset, MapTile(0, 0, 0, 0));
        }
        return _cachedRootNode.get();
    }

    const PackageTileMask::TileNode* PackageTileMask::findTileNode(const MapTile& tile) const {
        if (tile.getZoom() == 0) {
            if (tile.getX() == 0 && tile.getY() == 0) {
                return getRootNode();
            }
            return nullptr;
        }

        if (const TileNode* parentNode = findTileNode(MapTile(tile.getX() / 2, tile.getY() / 2, tile.getZoom() - 1, tile.getFrameNr()))) {
            if (parentNode->subNodes) {
                for (int idx = 0; idx < 4; idx++) {
                    const TileNode* node = &(*parentNode->subNodes)[idx];
                    if (node->zoom == tile.getZoom() && node->x == tile.getX() && node->y == tile.getY()) {
                        return node;
                    }
                }
            }
            if (parentNode->inside) {
                return parentNode;
            }
        }
        return nullptr;
    }

    void PackageTileMask::BuildTileNode(TileNode& node, const std::unordered_set<MapTile>& tileSet, const MapTile& tile, int clipZoom) {
        node.x = tile.getX();
        node.y = tile.getY();
        node.zoom = tile.getZoom();
        node.inside = (tileSet.find(tile) != tileSet.end() ? 1 : 0);
        if (!node.inside || node.zoom >= clipZoom) {
            return; // Note: we assume here that tile does not exist implies subtiles do not exist
        }

        node.subNodes = std::make_unique<std::array<TileNode, 4> >();
        for (int idx = 0; idx < 4; idx++) {
            int dx = idx % 2;
            int dy = idx / 2;
            BuildTileNode((*node.subNodes)[idx], tileSet, MapTile(tile.getX() * 2 + dx, tile.getY() * 2 + dy, tile.getZoom() + 1, tile.getFrameNr()), clipZoom);
        }

        bool keepSubnodes = false;
        for (int idx = 0; idx < 4; idx++) {
            const TileNode& subNode = (*node.subNodes)[idx];
            if (!subNode.inside || subNode.subNodes) {
                keepSubnodes = true;
            }
        }
        if (!keepSubnodes) {
            node.subNodes.reset();
        }
    }
    
    void PackageTileMask::DecodeTileNode(TileNode& node, const std::vector<bool>& data, std::size_t& offset, const MapTile& tile) {
        bool subNodes = data.at(offset++);
        bool inside = data.at(offset++);
        if (subNodes) {
            node.subNodes = std::make_unique<std::array<TileNode, 4> >();
            for (int idx = 0; idx < 4; idx++) {
                int dx = idx % 2;
                int dy = idx / 2;
                DecodeTileNode((*node.subNodes)[idx], data, offset, MapTile(tile.getX() * 2 + dx, tile.getY() * 2 + dy, tile.getZoom() + 1, tile.getFrameNr()));
            }
        }
        node.x = tile.getX();
        node.y = tile.getY();
        node.zoom = tile.getZoom();
        node.inside = inside ? 1 : 0;
    }

    void PackageTileMask::EncodeTileNode(const TileNode& node, std::vector<bool>& data) {
        data.push_back(node.subNodes ? true : false);
        data.push_back(node.inside ? true : false);
        if (node.subNodes) {
            for (int idx = 0; idx < 4; idx++) {
                EncodeTileNode((*node.subNodes)[idx], data);
            }
        }
    }

    std::vector<std::vector<MapPos> > PackageTileMask::CalculateTileNodeBoundingPolygon(const TileNode& node, const std::shared_ptr<Projection>& proj) {
        std::vector<std::vector<MapPos> > poly;
        if (node.subNodes) {
            for (int idx = 0; idx < 4; idx++) {
                std::vector<std::vector<MapPos> > subPoly = CalculateTileNodeBoundingPolygon((*node.subNodes)[idx], proj);
                if (!poly.empty() && !subPoly.empty()) {
                    poly = unifyTilePolygons(poly, subPoly);
                } else if (!subPoly.empty()) {
                    poly = std::move(subPoly);
                }
            }
        }

        if (poly.empty() && node.inside) {
            poly = createTilePolygon(MapTile(node.x, node.y, node.zoom, 0), proj);
        }
        return poly;
    }

}
