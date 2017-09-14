#include "GeneralUtils.h"

#include <sstream>

namespace carto {

    long long GeneralUtils::IntPow(int x, unsigned int p) {
        if (p == 0) return 1;
        if (p == 1) return x;
    
        long long tmp = IntPow(x, p/2);
        if (p % 2 == 0) {
            return tmp * tmp;
        } else {
            return x * tmp * tmp;
        }
    }
    
    unsigned int GeneralUtils::UpperPow2(unsigned int n) {
        unsigned int p = 1;
        while (p < n) {
            p <<= 1;
        }
        return p;
    }
    
    bool GeneralUtils::IsPow2(unsigned int n) {
        return (n != 0) && ((n & (n - 1)) == 0);
    }
    
    void GeneralUtils::ReplaceSubstrings(std::string& base, const std::string& search, const std::string& replace) {
        for (std::size_t pos = 0;; pos += replace.length()) {
            pos = base.find(search, pos);
            if (pos == std::string::npos) {
                break;
            }
            base.erase(pos, search.length());
            base.insert(pos, replace);
        }
    }
 
    std::string GeneralUtils::ReplaceTags(const std::string& base, const std::map<std::string, std::string>& tagValues, const std::string& startTag, const std::string& endTag, bool keepUnknownTags) {
        std::string output;
        std::string::size_type pos = 0;
        while (pos < base.size()) {
            std::string::size_type startPos = base.find(startTag, pos);
            if (startPos == std::string::npos) {
                break;
            }
            std::string::size_type endPos = base.find(endTag, startPos + startTag.size());
            if (endPos == std::string::npos) {
                break;
            }
            output += base.substr(pos, startPos - pos);
            std::string tag = base.substr(startPos + startTag.size(), endPos - startPos - startTag.size());
            auto it = tagValues.find(tag);
            if (it != tagValues.end()) {
                output += it->second;
            } else if (keepUnknownTags) {
                output += base.substr(startPos, endPos + endTag.size() - startPos);
            }
            pos = endPos + endTag.size();
        }
        output += base.substr(pos);
        return output;
    }
        
    std::vector<std::string>& GeneralUtils::Split(const std::string& str, char delim, std::vector<std::string>& elems) {
        std::stringstream ss(str);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }
    
    std::vector<std::string> GeneralUtils::Split(const std::string& str, char delim) {
        std::vector<std::string> elems;
        Split(str, delim, elems);
        return elems;
    }

    std::string GeneralUtils::Join(const std::vector<std::string>& strs, char delim) {
        std::string str;
        for (std::size_t i = 0; i < strs.size(); i++) {
            if (i > 0) {
                str += delim;
            }
            str += strs[i];
        }
        return str;
    }

    GeneralUtils::GeneralUtils() {
    }

}
