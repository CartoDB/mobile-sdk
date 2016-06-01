/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CLASSREGISTRY_H_
#define _CARTO_CLASSREGISTRY_H_

#include <typeinfo>
#include <typeindex>
#include <mutex>
#include <string>
#include <unordered_map>

namespace carto {

    class ClassRegistry {
    public:
        struct Entry {
            explicit Entry(const std::type_info& typeInfo, const char* name);
        };

        virtual ~ClassRegistry();

        static std::string GetClassName(const std::type_info& typeInfo);

    private:
        ClassRegistry();

        static ClassRegistry& GetInstance();

        std::unordered_map<std::type_index, std::string> _classNames;
        std::mutex _mutex;
    };

}

#endif
