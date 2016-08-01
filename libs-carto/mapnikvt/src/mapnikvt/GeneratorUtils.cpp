#include "GeneratorUtils.h"
#include "ValueGenerator.h"
#include "ExpressionGenerator.h"
#include "TransformGenerator.h"
#include "ColorGenerator.h"

#include <iomanip>
#include <sstream>
#include <utility>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace carto { namespace mvt {
    std::string generateColorString(vt::Color color) {
        std::string str;
        std::back_insert_iterator<std::string> it(str);
        colorgenimpl::encoding::space_type space;
        bool result = boost::spirit::karma::generate_delimited(it, ColorGenerator<std::back_insert_iterator<std::string>>(), space, color.value());
        if (!result) {
            throw GeneratorException("Could not generate color string");
        }
        return boost::trim_copy(str);
    }

    std::string generateValueString(const Value& val) {
        std::string str;
        std::back_insert_iterator<std::string> it(str);
        valgenimpl::encoding::space_type space;
        bool result = boost::spirit::karma::generate_delimited(it, ValueGenerator<std::back_insert_iterator<std::string>>(), space, val);
        if (!result) {
            throw GeneratorException("Could not generate value string");
        }
        return boost::trim_copy(str);
    }

    std::string generateExpressionString(const std::shared_ptr<const Expression>& expr) {
        std::string str;
        std::back_insert_iterator<std::string> it(str);
        exprgenimpl::encoding::space_type space;
        bool result = boost::spirit::karma::generate_delimited(it, ExpressionGenerator<std::back_insert_iterator<std::string>>(), space, expr);
        if (!result) {
            throw GeneratorException("Could not generate expression string");
        }
        return boost::trim_copy(str);
    }

    std::string generateStringExpressionString(const std::shared_ptr<const Expression>& expr) {
        std::string str;
        std::back_insert_iterator<std::string> it(str);
        exprgenimpl::encoding::space_type space;
        bool result = boost::spirit::karma::generate_delimited(it, StringExpressionGenerator<std::back_insert_iterator<std::string>>(), space, expr);
        if (!result) {
            throw GeneratorException("Could not generate string expression string");
        }
        return boost::trim_copy(str);
    }

    std::string generateTransformListString(const std::vector<std::shared_ptr<const Transform>>& transforms) {
        std::string str;
        std::back_insert_iterator<std::string> it(str);
        transgenimpl::encoding::space_type space;
        bool result = boost::spirit::karma::generate_delimited(it, TransformGenerator<std::back_insert_iterator<std::string>>() % ',', space, transforms);
        if (!result) {
            throw GeneratorException("Could not generate transform string");
        }
        return boost::trim_copy(str);
    }
} }
