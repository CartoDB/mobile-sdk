#include "QueryExpressionParser.h"
#include "components/Exceptions.h"
#include "search/query/QueryExpression.h"
#include "search/query/QueryExpressionImpl.h"
#include "utils/Log.h"

#include <memory>
#include <functional>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace carto {
    namespace queryexpressionimpl {
        namespace phx = boost::phoenix;
        namespace qi = boost::spirit::qi;
        namespace repo = boost::spirit::repository::qi;
        namespace encoding = boost::spirit::iso8859_1;
    
        template <typename Iterator>
        struct Grammar : qi::grammar<Iterator, std::shared_ptr<QueryExpression>(), encoding::space_type> {
            Grammar() : Grammar::base_type(expression) {
                using qi::_val;
                using qi::_1;
                using qi::_2;

                unesc_char.add("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
                ("\\r", '\r')("\\t", '\t')("\\v", '\v')("\\\\", '\\')
                ("\\\'", '\'')("\\\"", '\"');

                is_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["is"]];
                or_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["or"]];
                and_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["and"]];
                not_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["not"]];
                null_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["null"]];
                false_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["false"]];
                true_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["true"]];
                collate_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["collate"]];
                regexp_like_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["regexp_like"]];

                string =
                    '\'' >> *(unesc_char | "\\x" >> qi::hex | (qi::print - '\'')) >> '\'';

                value =
                      null_kw       [_val = phx::construct<Value>()]
                    | false_kw      [_val = phx::construct<Value>(false)]
                    | true_kw       [_val = phx::construct<Value>(true)]
                    | qi::long_long [_val = phx::construct<Value>(_1)]
                    | qi::double_   [_val = phx::construct<Value>(_1)]
                    | string        [_val = phx::construct<Value>(_1)]
                    ;

                identifier %=
                    qi::lexeme[qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z0-9_")] >> -(qi::string("::") >> identifier)
                    ;

                expression =
                    term1 [_val = _1] >> *((("||" | or_kw) > term1) [_val = phx::bind(&OrExpression::create, _val, _1)])
                    ;

                term1 =
                    term2 [_val = _1] >> *((("&&" | and_kw) > term1) [_val = phx::bind(&AndExpression::create, _val, _1)])
                    ;

                term2 =
                      term3 [_val = _1]
                    | (("!" || not_kw) >> term3) [_val = phx::bind(&NotExpression::create, _1)]
                    ;

                term3 =
                      predicate [_val = _1]
                    | (regexp_like_kw >> '(' > operand > ',' > operand > ')')  [_val = phx::bind(&BinaryPredicateExpression<RegexpLikePredicate>::create, _1, _2)]
                    | ('(' >> expression > ')') [_val = _1]
                    ;

                predicate =
                      (operand >> "<=" > operand) [_val = phx::bind(&BinaryPredicateExpression<LtePredicate>::create, _1, _2)]
                    | (operand >> ">=" > operand) [_val = phx::bind(&BinaryPredicateExpression<GtePredicate>::create, _1, _2)]
                    | (operand >> "<"  > operand) [_val = phx::bind(&BinaryPredicateExpression<LtPredicate>::create, _1, _2)]
                    | (operand >> ">"  > operand) [_val = phx::bind(&BinaryPredicateExpression<GtPredicate>::create, _1, _2)]
                    | (operand >> (qi::lit("==") | qi::lit("=") ) > operand) [_val = phx::bind(&BinaryPredicateExpression<EqPredicate>::create, _1, _2)]
                    | (operand >> (qi::lit("<>") | qi::lit("!=")) > operand) [_val = phx::bind(&BinaryPredicateExpression<NeqPredicate>::create, _1, _2)]
                    | (operand >> is_kw >> null_kw)           [_val = phx::bind(&UnaryPredicateExpression<IsNullPredicate>::create, _1)]
                    | (operand >> is_kw >> not_kw >> null_kw) [_val = phx::bind(&UnaryPredicateExpression<IsNotNullPredicate>::create, _1)]
                    ;
        
                operand =
                      value [_val = phx::bind(&ConstOperand::create, _1)]
                    | (identifier >> collate_kw > identifier) [_val = phx::bind(&VariableOperand::createEx, _1, _2)]
                    | identifier [_val = phx::bind(&VariableOperand::create, _1)]
                    ;
            }

            qi::symbols<char const, char const> unesc_char;
            qi::rule<Iterator, qi::unused_type()> is_kw, or_kw, and_kw, not_kw, null_kw, false_kw, true_kw, collate_kw, regexp_like_kw;
            qi::rule<Iterator, std::string()> string;
            qi::rule<Iterator, Value()> value;
            qi::rule<Iterator, std::string()> identifier;
            qi::rule<Iterator, std::shared_ptr<Expression>(), encoding::space_type> expression, term1, term2, term3, predicate;
            qi::rule<Iterator, std::shared_ptr<Operand>(), encoding::space_type> operand;
        };
    }

    std::shared_ptr<QueryExpression> QueryExpressionParser::parse(const std::string& expr) {
        std::string::const_iterator it = expr.begin();
        std::string::const_iterator end = expr.end();
        queryexpressionimpl::encoding::space_type space;
        std::shared_ptr<QueryExpression> queryExpr;
        bool result = boost::spirit::qi::phrase_parse(it, end, queryexpressionimpl::Grammar<std::string::const_iterator>(), space, queryExpr);
        if (!result) {
            throw ParseException("Failed to parse query expression", expr);
        } else if (it != expr.end()) {
            throw ParseException("Could not parse to the end of query expression", expr, static_cast<int>(expr.end() - it));
        }
        return queryExpr;
    }

    QueryExpressionParser::QueryExpressionParser() {
    }

}
