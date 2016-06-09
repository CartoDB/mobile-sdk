#include "UTFGridTile.h"
#include "core/BinaryData.h"
#include "utils/Log.h"

#include <boost/lexical_cast.hpp>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <utf8.h>

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

        std::map<std::string, std::map<std::string, std::string> > data;
        if (doc.FindMember("data") != doc.MemberEnd()) {
            for (rapidjson::Value::ConstMemberIterator it = doc["data"].MemberBegin(); it != doc["data"].MemberEnd(); ++it) {
                if (!it->name.IsString() || !it->value.IsObject()) {
                    continue;
                }
                std::string key = it->name.GetString();
                for (rapidjson::Value::ConstMemberIterator it2 = it->value.MemberBegin(); it2 != it->value.MemberEnd(); it2++) {
                    const rapidjson::Value* value = &it2->value;
                    if (!it2->name.IsString()) {
                        continue;
                    }
                    std::string str;
                    if (value->IsString()) {
                        str = value->GetString();
                    }
                    else if (value->IsInt()) {
                        str = boost::lexical_cast<std::string>(value->GetInt());
                    }
                    else if (value->IsUint()) {
                        str = boost::lexical_cast<std::string>(value->GetUint());
                    }
                    else if (value->IsInt64()) {
                        str = boost::lexical_cast<std::string>(value->GetInt64());
                    }
                    else if (value->IsUint64()) {
                        str = boost::lexical_cast<std::string>(value->GetUint64());
                    }
                    else if (value->IsNumber()) {
                        str = boost::lexical_cast<std::string>(value->GetDouble());
                    }
                    data[key][it2->name.GetString()] = str;
                }
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
