/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GENERALUTILS_H_
#define _CARTO_GENERALUTILS_H_

#include <string>
#include <vector>
#include <map>

namespace carto {

    class GeneralUtils {
    public:
        template <typename T>
        static T Clamp(const T& value, const T& low, const T& high) {
            return value < low ? low : (value > high ? high : value);
        }
    
        static long long IntPow(int x, unsigned int p);
    
        static unsigned int UpperPow2(unsigned int n);
    
        static bool IsPow2(unsigned int n);
    
        static void ReplaceSubstrings(std::string& base, const std::string& search, const std::string& replace);
        
        static std::string ReplaceTags(const std::string& base, const std::map<std::string, std::string>& tagValues, const std::string& startTag = "{", const std::string& endTag = "}", bool keepUnknownTags = false);

        static std::vector<std::string>& Split(const std::string& s, char delim, std::vector<std::string>& elems);
        
        static std::vector<std::string> Split(const std::string& s, char delim);

    private:
        GeneralUtils();
    };
    
}

#endif
