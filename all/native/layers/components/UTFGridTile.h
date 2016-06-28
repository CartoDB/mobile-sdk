/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_UTFGRIDTILE_H_
#define _CARTO_UTFGRIDTILE_H_

#include "core/Variant.h"

#include <memory>
#include <map>
#include <vector>
#include <string>

namespace carto {
    class BinaryData;
        
    class UTFGridTile {
    public:
        UTFGridTile(const std::vector<std::string>& keys, const std::map<std::string, Variant>& data, const std::vector<int>& keyIds, int xSize, int ySize) : _keys(keys), _data(data), _keyIds(keyIds), _xSize(xSize), _ySize(ySize) { }

        std::string getKey(int keyId) const {
            return keyId >= 0 && keyId <= static_cast<int>(_keys.size()) ? _keys[keyId] : std::string();
        }
        
        Variant getData(const std::string& key) const {
            auto it = _data.find(key);
            return it != _data.end() ? it->second : Variant();
        }

        int getXSize() const {
            return _xSize;
        }
        
        int getYSize() const {
            return _ySize;
        }
        
        int getKeyId(int x, int y) const {
            return x >= 0 && y >= 0 && x < getXSize() && y < getYSize() ? _keyIds[y * getXSize() + x] : 0;
        }

        static std::shared_ptr<UTFGridTile> DecodeUTFTile(const std::shared_ptr<BinaryData>& tileData);

    private:
        std::vector<std::string> _keys;
        std::map<std::string, Variant> _data;
        std::vector<int> _keyIds;
        int _xSize;
        int _ySize;
    };
    
}

#endif
