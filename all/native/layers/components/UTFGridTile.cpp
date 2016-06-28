#include "UTFGridTile.h"
#include "core/BinaryData.h"
#include "utils/Log.h"

#include <boost/lexical_cast.hpp>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <utf8.h>

namespace {

    carto::Variant rapidJSONToVariant(const rapidjson::Value& value) {
        if (value.IsString()) {
            return carto::Variant(value.GetString());
        }
        else if (value.IsInt()) {
            return carto::Variant(static_cast<long long>(value.GetInt()));
        }
        else if (value.IsUint()) {
            return carto::Variant(static_cast<long long>(value.GetUint()));
        }
        else if (value.IsInt64()) {
            return carto::Variant(static_cast<long long>(value.GetInt64()));
        }
        else if (value.IsUint64()) {
            return carto::Variant(static_cast<long long>(value.GetUint64()));
        }
        else if (value.IsNumber()) {
            return carto::Variant(value.GetDouble());
        }
        else if (value.IsArray()) {
            std::vector<carto::Variant> values;
            for (rapidjson::Value::ConstValueIterator it = value.Begin(); it != value.End(); it++) {
                values.push_back(rapidJSONToVariant(*it));
            }
            return carto::Variant(values);
        }
        else if (value.IsObject()) {
            std::map<std::string, carto::Variant> valueMap;
            for (rapidjson::Value::ConstMemberIterator it = value.MemberBegin(); it != value.MemberEnd(); it++) {
                const rapidjson::Value* value = &it->value;
                if (!it->name.IsString()) {
                    continue;
                }
          
                valueMap[it->name.GetString()] = rapidJSONToVariant(it->value);
            }
            return carto::Variant(valueMap);
        }
        return carto::Variant();
    }

}

namespace carto {

    std::shared_ptr<UTFGridTile> UTFGridTile::DecodeUTFTile(const std::shared_ptr<BinaryData>& tileData) {
        if (!tileData) {
            Log::Error("UTFGridTile::DecodeUTFTile: Null tile data");
            return std::shared_ptr<UTFGridTile>();
        }

        std::string json(reinterpret_cast<const char*>(tileData->data()), tileData->size());
        rapidjson::Document doc;
        if (doc.Parse<rapidjson::kParseDefaultFlags>(json.c_str()).HasParseError()) {
            Log::Error("UTFGridTile::DecodeUTFTile: Failed to parse JSON");
            return std::shared_ptr<UTFGridTile>();
        }

        std::vector<std::string> keys;
        for (unsigned int i = 0; i < doc["keys"].Size(); i++) {
            keys.push_back(doc["keys"][i].GetString());
        }

        std::map<std::string, Variant> data;
        if (doc.FindMember("data") != doc.MemberEnd()) {
            for (rapidjson::Value::ConstMemberIterator it = doc["data"].MemberBegin(); it != doc["data"].MemberEnd(); ++it) {
                if (!it->name.IsString()) {
                    continue;
                }

                data[it->name.GetString()] = rapidJSONToVariant(it->value);
            }
        }

        unsigned int rows = doc["grid"].Size();
        unsigned int cols = 0;
        for (unsigned int i = 0; i < rows; i++) {
            std::string columnUTF8 = doc["grid"][i].GetString();
            std::vector<std::uint32_t> column;
            column.reserve(columnUTF8.size());
            utf8::utf8to32(columnUTF8.begin(), columnUTF8.end(), std::back_inserter(column));

            cols = std::max(cols, static_cast<unsigned int>(column.size()));
        }
        std::vector<int> keyIds;
        keyIds.reserve(cols * rows);
        for (unsigned int i = 0; i < rows; i++) {
            std::string columnUTF8 = doc["grid"][i].GetString();
            std::vector<std::uint32_t> column;
            column.reserve(columnUTF8.size());
            utf8::utf8to32(columnUTF8.begin(), columnUTF8.end(), std::back_inserter(column));

            if (column.size() != cols) {
                Log::Warnf("UTFGridTile::DecodeUTFTile: Mismatching rows/columns");
                column.resize(cols - column.size(), ' ');
            }

            for (std::size_t j = 0; j < column.size(); j++) {
                std::uint32_t code = column[j];
                if (code >= 93) code--;
                if (code >= 35) code--;
                code -= 32;
                keyIds.push_back(code);
            }
        }
        return std::make_shared<UTFGridTile>(keys, data, keyIds, cols, rows);
    }

}
