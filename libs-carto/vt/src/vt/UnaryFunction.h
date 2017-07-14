/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_UNARYFUNCTION_H_
#define _CARTO_VT_UNARYFUNCTION_H_

#include <memory>
#include <functional>

namespace carto { namespace vt {
    template <typename Result, typename Argument>
    class UnaryFunction final {
    public:
        using Function = std::shared_ptr<std::function<Result(const Argument&)>>;
        
        UnaryFunction() : _func(), _value() { }
        explicit UnaryFunction(Result val) : _func(), _value(val) { }
        explicit UnaryFunction(Function func) : _func(std::move(func)), _value() { }

        const Result& value() const { return _value; }
        const Function& function() const { return _func; }

        Result operator() (const Argument& arg) const {
            if (!_func) {
                return _value;
            }
            return (*_func)(arg);
        }

    private:
        Function _func;
        Result _value;
    };

    template <typename Result, typename Argument>
    bool operator == (const UnaryFunction<Result, Argument>& func1, const UnaryFunction<Result, Argument>& func2) {
        return func1.value() == func2.value() && func1.function() == func2.function();
    }
    
    template <typename Result, typename Argument>
    bool operator != (const UnaryFunction<Result, Argument>& func1, const UnaryFunction<Result, Argument>& func2) {
        return !(func1 == func2);
    }
} }

namespace std {
    template <typename Result, typename Argument>
    struct hash<carto::vt::UnaryFunction<Result, Argument>> {
        std::size_t operator() (const carto::vt::UnaryFunction<Result, Argument>& func) const {
            return std::hash<Result>()(func.value()) + std::hash<void*>()(func.function().get()) * 2;
        }
    };
}

#endif
