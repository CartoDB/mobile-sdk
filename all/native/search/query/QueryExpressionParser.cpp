#include "QueryExpressionParser.h"
#include "components/Exceptions.h"
#include "search/query/QueryExpression.h"
#include "search/query/QueryExpressionImpl.h"
#include "utils/Log.h"

#include <memory>
#include <functional>

#include <boost/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>

namespace carto {
    namespace queryexpressionimpl {
        using Skipper = boost::spirit::iso8859_1::space_type;
    
        template <typename Iterator>
        struct Grammar : boost::spirit::qi::grammar<Iterator, std::shared_ptr<QueryExpression>(), Skipper> {
            Grammar() : Grammar::base_type(expression) {
                using namespace boost;
                using namespace boost::spirit;
                using qi::_val;
                using qi::_1;
                using qi::_2;

                unesc_char.add("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
                              ("\\r", '\r')("\\t", '\t')("\\v", '\v')("\\\\", '\\')
                              ("\\\'", '\'')("\\\"", '\"');

                is_kw    = repository::qi::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["is"]];
                or_kw    = repository::qi::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["or"]];
                and_kw   = repository::qi::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["and"]];
                not_kw   = repository::qi::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["not"]];
                null_kw  = repository::qi::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["null"]];
                false_kw = repository::qi::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["false"]];
                true_kw  = repository::qi::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["true"]];
                collate_kw = repository::qi::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["collate"]];
                regexp_like_kw  = repository::qi::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["regexp_like"]];
                regexp_ilike_kw = repository::qi::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["regexp_ilike"]];

                string =
                    '\'' >> *(unesc_char | "\\x" >> qi::hex | (qi::char_ - '\'')) >> '\'';

                value =
                      null_kw       [_val = phoenix::construct<Value>()]
                    | false_kw      [_val = phoenix::construct<Value>(false)]
                    | true_kw       [_val = phoenix::construct<Value>(true)]
                    | qi::long_long [_val = phoenix::construct<Value>(_1)]
                    | qi::double_   [_val = phoenix::construct<Value>(_1)]
                    | string        [_val = phoenix::construct<Value>(_1)]
                    ;

                identifier %=
                    qi::lexeme[qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z0-9_")] >> -(qi::string("::") >> identifier)
                    ;

                expression =
                    term1 [_val = _1] >> *((("||" | or_kw) > term1) [_val = phoenix::bind(&OrExpression::create, _val, _1)])
                    ;

                term1 =
                    term2 [_val = _1] >> *((("&&" | and_kw) > term1) [_val = phoenix::bind(&AndExpression::create, _val, _1)])
                    ;

                term2 =
                      term3 [_val = _1]
                    | (("!" || not_kw) >> term3) [_val = phoenix::bind(&NotExpression::create, _1)]
                    ;

                term3 =
                      predicate [_val = _1]
                    | (regexp_like_kw  >> '(' > operand > ',' > operand > ')')  [_val = phoenix::bind(&BinaryPredicateExpression<RegexpLikePredicate<false>>::create, _1, _2)]
                    | (regexp_ilike_kw >> '(' > operand > ',' > operand > ')')  [_val = phoenix::bind(&BinaryPredicateExpression<RegexpLikePredicate<true>>::create, _1, _2)]
                    | ('(' >> expression > ')') [_val = _1]
                    ;

                predicate =
                      (operand >> "<=" > operand) [_val = phoenix::bind(&BinaryPredicateExpression<LtePredicate>::create, _1, _2)]
                    | (operand >> ">=" > operand) [_val = phoenix::bind(&BinaryPredicateExpression<GtePredicate>::create, _1, _2)]
                    | (operand >> "<"  > operand) [_val = phoenix::bind(&BinaryPredicateExpression<LtPredicate>::create, _1, _2)]
                    | (operand >> ">"  > operand) [_val = phoenix::bind(&BinaryPredicateExpression<GtPredicate>::create, _1, _2)]
                    | (operand >> (qi::lit("==") | qi::lit("=") ) > operand) [_val = phoenix::bind(&BinaryPredicateExpression<EqPredicate>::create, _1, _2)]
                    | (operand >> (qi::lit("<>") | qi::lit("!=")) > operand) [_val = phoenix::bind(&BinaryPredicateExpression<NeqPredicate>::create, _1, _2)]
                    | (operand >> is_kw >> null_kw)           [_val = phoenix::bind(&UnaryPredicateExpression<IsNullPredicate>::create, _1)]
                    | (operand >> is_kw >> not_kw >> null_kw) [_val = phoenix::bind(&UnaryPredicateExpression<IsNotNullPredicate>::create, _1)]
                    ;
        
                operand =
                      value [_val = phoenix::bind(&ConstOperand::create, _1)]
                    | (identifier >> collate_kw > identifier) [_val = phoenix::bind(&VariableOperand::createEx, _1, _2)]
                    | identifier [_val = phoenix::bind(&VariableOperand::create, _1)]
                    ;
            }

            boost::spirit::qi::symbols<char const, char const> unesc_char;
            boost::spirit::qi::rule<Iterator, boost::spirit::qi::unused_type()> is_kw, or_kw, and_kw, not_kw, null_kw, false_kw, true_kw, collate_kw, regexp_like_kw, regexp_ilike_kw;
            boost::spirit::qi::rule<Iterator, std::string()> string;
            boost::spirit::qi::rule<Iterator, Value()> value;
            boost::spirit::qi::rule<Iterator, std::string()> identifier;
            boost::spirit::qi::rule<Iterator, std::shared_ptr<Expression>(), Skipper> expression, term1, term2, term3, predicate;
            boost::spirit::qi::rule<Iterator, std::shared_ptr<Operand>(), Skipper> operand;
        };
    }

    std::shared_ptr<QueryExpression> QueryExpressionParser::parse(const std::string& expr) {
        std::string::const_iterator it = expr.begin();
        std::string::const_iterator end = expr.end();
        std::shared_ptr<QueryExpression> queryExpr;
        bool result = false;
        try {
            queryexpressionimpl::Skipper skipper;
            result = boost::spirit::qi::phrase_parse(it, end, queryexpressionimpl::Grammar<std::string::const_iterator>(), skipper, queryExpr);
        }
        catch (const boost::spirit::qi::expectation_failure<std::string::const_iterator>& ex) {
            throw ParseException("Expectation error", expr, static_cast<int>(ex.first - expr.begin()));
        }
        if (!result) {
            throw ParseException("Failed to parse query expression", expr);
        } else if (it != expr.end()) {
            throw ParseException("Could not parse to the end of query expression", expr, static_cast<int>(it - expr.begin()));
        }
        return queryExpr;
    }

    QueryExpressionParser::QueryExpressionParser() {
    }

}
