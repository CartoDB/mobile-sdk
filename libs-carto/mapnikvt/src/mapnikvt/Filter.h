/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_FILTER_H_
#define _CARTO_MAPNIKVT_FILTER_H_

#include <memory>

namespace carto { namespace mvt {
    class Predicate;

    class Filter final {
    public:
        enum class Type {
            FILTER,
            ELSEFILTER,
            ALSOFILTER
        };

        explicit Filter(Type type, std::shared_ptr<const Predicate> pred) : _type(type), _pred(std::move(pred)) { }

        Type getType() const { return _type; }
        const std::shared_ptr<const Predicate>& getPredicate() const { return _pred; }

    private:
        const Type _type;
        const std::shared_ptr<const Predicate> _pred;
    };
} }

#endif
