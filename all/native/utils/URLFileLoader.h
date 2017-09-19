/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_URLFILELOADER_H_
#define _CARTO_URLFILELOADER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>

namespace carto {
    class BinaryData;
    
    class URLFileLoader {
    public:
        typedef std::function<bool(std::uint64_t, const unsigned char*, std::size_t)> HandlerFunc;

        URLFileLoader(const std::string& tag, bool cacheFiles);
        virtual ~URLFileLoader();

        bool loadFile(const std::string& url, std::shared_ptr<BinaryData>& data) const;
        bool streamFile(const std::string& url, HandlerFunc handler) const;
        
    private:
        const std::string _tag;
        const bool _cacheFiles;
        mutable std::map<std::string, std::shared_ptr<BinaryData> > _cachedFiles;
        mutable std::mutex _mutex;
    };

}

#endif
