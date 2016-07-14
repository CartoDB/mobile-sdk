#include "CartoCSSParser.h"
#include "Expression.h"
#include "Predicate.h"
#include "Mapnikvt/CSSColorParser.h"

#include <regex>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace carto { namespace css {
    namespace cssparserimpl {
        namespace phx = boost::phoenix;
        namespace qi = boost::spirit::qi;
        namespace encoding = boost::spirit::iso8859_1;
        
        template <typename Iterator>
        struct Skipper : public qi::grammar<Iterator> {
            Skipper() : Skipper::base_type(skip_, "PL/0") {
                skip_ = qi::iso8859_1::space | ("/*" >> *(qi::char_ - "*/") >> "*/") | ("//" >> *(qi::char_ - qi::char_("\n\r")));
            }
            qi::rule<Iterator> skip_;
        };
        
        template <typename Iterator>
        struct Grammar : qi::grammar<Iterator, StyleSheet(), Skipper<Iterator>> {
            Grammar() : Grammar::base_type(stylesheet) {
                using qi::_val;
                using qi::_1;
                using qi::_2;
                using qi::_3;
                using qi::_pass;
                
                unesc_char.add
                    ("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
                    ("\\r", '\r')("\\t", '\t')("\\v", '\v')("\\\\", '\\')
                    ("\\\'", '\'')("\\\"", '\"');
                
                nonascii_  = qi::char_("\xA0-\xFF");
                nmstart_   = qi::char_("_a-zA-Z-") | nonascii_;
                nmchar_    = qi::char_("_a-zA-Z0-9-") | nonascii_;
                unescaped_ = qi::char_("_a-zA-Z0-9-#().,%");
                hex_       = *qi::char_("0-9a-fA-F");

                string =
                      '\'' >> *(unesc_char | "\\x" >> octet_ | (qi::print - '\'')) >> '\''
                    | '\"' >> *(unesc_char | "\\x" >> octet_ | (qi::print - '\"')) >> '\"'
                    ;
                
                literal =
                      (qi::char_("/_a-zA-Z-") | nonascii_) > *(qi::char_("/_a-zA-Z0-9-") | nonascii_)
                    ;

                uri =
                      qi::lit("url") >> '(' >> string > ')'
                    | qi::lit("url") >> '(' >> *(unesc_char | "\\x" >> octet_ | (qi::print - ')')) > ')'
                    ;

                color =
                      qi::lit('#') > (*qi::char_("0-9A-Fa-f"))      [_pass = phx::bind(&getHEXColor, _val, _1)]
                    | (*qi::char_("a-z"))                           [_pass = phx::bind(&getCSSColor, _val, _1)]
                    ;

                number =
                      (qi::real_parser<double, qi::real_policies<double>>() >> *qi::space >> '%') [_val = phx::construct<Value>(_1 / 100.0)]
                    | (qi::real_parser<double, qi::real_policies<double>>() >> *qi::space >> "px") [_val = phx::construct<Value>(_1)]
                    | qi::real_parser<double, qi::strict_real_policies<double>>() [_val = phx::construct<Value>(_1)]
                    | qi::long_long                                 [_val = phx::construct<Value>(_1)]
                    ;

                constant =
                      qi::lit("null")                               [_val = phx::construct<Value>()]
                    | qi::lit("false")                              [_val = phx::construct<Value>(false)]
                    | qi::lit("true")                               [_val = phx::construct<Value>(true)]
                    | number                                        [_val = _1]
                    | uri                                           [_val = phx::construct<Value>(_1)]
                    | color                                         [_val = phx::construct<Value>(_1)]
                    | string                                        [_val = phx::construct<Value>(_1)]
                    | literal                                       [_val = phx::construct<Value>(_1)]
                    ;
                
                propid = qi::lexeme[(qi::char_("/_a-zA-Z-") | nonascii_) > *(qi::char_("/_a-zA-Z0-9-") | nonascii_)];
                blockid = qi::lexeme[+(qi::char_("_a-zA-Z0-9-") | nonascii_)];
                fieldid = qi::lexeme[+(qi::char_("_a-zA-Z0-9-") | nonascii_)];
                unescapedfieldid = qi::lexeme[+(qi::print - '[' - ']')];
                varid = qi::lexeme[+(qi::char_("_a-zA-Z0-9-") | nonascii_)];
                funcid = (nmstart_ > *nmchar_) [_pass = phx::bind(&makeIdentifier, _val, _1, _2)];

                expressionlist =
                      (expression >> (',' > (expression % ',')))   [_val = phx::bind(&makeListExpression, _1, _2)]
                    | expression                                    [_val = _1]
                    ;

                expression =
                    term											[_val = _1]
                    >> *( (qi::lit("+") >> term)					[_val = phx::bind(&makeBinaryExpression, BinaryExpression::Op::ADD, _val, _1)]
                        | (qi::lit("-") >> term)					[_val = phx::bind(&makeBinaryExpression, BinaryExpression::Op::SUB, _val, _1)]
                        )
                    ;
                
                term =
                    unary											[_val = _1]
                    >> *( (qi::lit("*") >> unary)					[_val = phx::bind(&makeBinaryExpression, BinaryExpression::Op::MUL, _val, _1)]
                        | (qi::lit("/") >> unary)					[_val = phx::bind(&makeBinaryExpression, BinaryExpression::Op::DIV, _val, _1)]
                        )
                    ;
                
                unary =
                      factor										[_val = _1]
                    | (qi::lit("-") >> unary)			            [_val = phx::bind(&makeUnaryExpression, UnaryExpression::Op::NEG, _1)]
                    ;

                factor =
                      ('@' > varid)					                [_val = phx::bind(&makeFieldVarExpression, false, _1)]
                    | ('[' > unescapedfieldid > ']')                [_val = phx::bind(&makeFieldVarExpression, true, _1)]
                    | (funcid >> ('(' > (expression % ',') > ')'))  [_val = phx::bind(&makeFunctionExpression, _1, _2)]
                    | ('(' > expression > ')')					    [_val = _1]
                    | constant									    [_val = phx::bind(&makeConstExpression, _1)]
                    ;
                
                op =
                     qi::lit("=~")                                  [_val = OpPredicate::Op::MATCH]
                   | qi::lit("=")                                   [_val = OpPredicate::Op::EQ]
                   | qi::lit("!=")                                  [_val = OpPredicate::Op::NEQ]
                   | qi::lit("<=")                                  [_val = OpPredicate::Op::LTE]
                   | qi::lit("<")                                   [_val = OpPredicate::Op::LT]
                   | qi::lit(">=")                                  [_val = OpPredicate::Op::GTE]
                   | qi::lit(">")                                   [_val = OpPredicate::Op::GT]
                   ;

                predicate =
                      qi::lit("Map")                                [_val = phx::bind(&makeMapPredicate)]
                    | (qi::lit('#') > blockid)                      [_val = phx::bind(&makeLayerPredicate, _1)]
                    | (qi::lit('.') > blockid)                      [_val = phx::bind(&makeClassPredicate, _1)]
                    | (qi::lit("::") > blockid)                     [_val = phx::bind(&makeAttachmentPredicate, _1)]
                    | ((qi::lit('[') >> '@') > varid > op > constant > ']') [_val = phx::bind(&makeOpPredicate, _2, false, _1, _3)]
                    | (qi::lit('[') > (fieldid | string) > op > constant > ']') [_val = phx::bind(&makeOpPredicate, _2, true, _1, _3)]
                    ;
                
                selector = (*predicate)                             [_val = phx::construct<Selector>(_1)];
                
                propdeclaration = (propid >> (':' > expressionlist)) [_val = phx::bind(&makePropertyDeclaration, phx::ref(_declarationOrder), _1, _2)];
                
                blockelement = (propdeclaration | ruleset)          [_val = phx::construct<Block::Element>(_1)];
                block = (*(blockelement > -qi::lit(';')))           [_val = phx::construct<Block>(_1)];
                
                ruleset = ((selector % ',') >> ('{' > block > '}')) [_val = phx::construct<RuleSet>(_1, _2)];
                
                vardeclaration = ('@' > varid > ':' > expressionlist) [_val = phx::construct<VariableDeclaration>(_1, _2)];

                stylesheetelement = (vardeclaration | ruleset)      [_val = phx::construct<StyleSheet::Element>(_1)];
                stylesheet = (*(stylesheetelement > -qi::lit(';'))) [_val = phx::construct<StyleSheet>(_1)];
            
                qi::on_error<qi::fail>(stylesheet, phx::ref(_errorPos) = qi::_3 - qi::_1);
            }

            std::string::size_type errorPos() const { return _errorPos; }
            
            qi::int_parser<char, 16, 2, 2> octet_;
            qi::symbols<char const, char const> unesc_char;
            qi::rule<Iterator, char()> nonascii_, nmstart_, nmchar_, unescaped_;
            qi::rule<Iterator, std::string()> hex_;
            qi::rule<Iterator, std::string()> string, literal, uri;
            qi::rule<Iterator, Color()> color;
            qi::rule<Iterator, Value()> number, constant;
            qi::rule<Iterator, std::string()> propid, blockid, fieldid, unescapedfieldid, varid, funcid;
            qi::rule<Iterator, std::shared_ptr<const Expression>(), Skipper<Iterator> > expressionlist, expression, term, unary, factor;
            qi::rule<Iterator, OpPredicate::Op()> op;
            qi::rule<Iterator, std::shared_ptr<const Predicate>(), Skipper<Iterator> > predicate;
            qi::rule<Iterator, Selector(), Skipper<Iterator> > selector;
            qi::rule<Iterator, PropertyDeclaration(), Skipper<Iterator> > propdeclaration;
            qi::rule<Iterator, Block::Element(), Skipper<Iterator> > blockelement;
            qi::rule<Iterator, Block(), Skipper<Iterator> > block;
            qi::rule<Iterator, RuleSet(), Skipper<Iterator> > ruleset;
            qi::rule<Iterator, VariableDeclaration(), Skipper<Iterator> > vardeclaration;
            qi::rule<Iterator, StyleSheet::Element(), Skipper<Iterator> > stylesheetelement;
            qi::rule<Iterator, StyleSheet(), Skipper<Iterator> > stylesheet;

        private:
            static bool getHEXColor(Color& color, const std::vector<char>& code) {
                unsigned int value = 0;
                if (!mvt::parseCSSColor("#" + std::string(code.begin(), code.end()), value)) {
                    return false;
                }
                color = Color::fromValue(value);
                return true;
            }

            static bool getCSSColor(Color& color, const std::vector<char>& name) {
                unsigned int value = 0;
                if (!mvt::parseCSSColor(std::string(name.begin(), name.end()), value)) {
                    return false;
                }
                color = Color::fromValue(value);
                return true;
            }

            static bool makeIdentifier(std::string& ident, char head, const std::vector<char>& tail) {
                ident = std::string(1, head) + std::string(tail.begin(), tail.end());
                return ident != "true" && ident != "false" && ident != "url";
            }
            
            static std::shared_ptr<const Expression> makeConstExpression(Value value) {
                return std::make_shared<ConstExpression>(value);
            }

            static std::shared_ptr<const Expression> makeFieldVarExpression(bool field, const std::string& var) {
                return std::make_shared<FieldOrVarExpression>(field, var);
            }
            
            static std::shared_ptr<const Expression> makeListExpression(const std::shared_ptr<const Expression>& initialExpr, const std::vector<std::shared_ptr<const Expression>>& restExprs) {
                std::vector<std::shared_ptr<const Expression>> exprs;
                exprs.push_back(initialExpr);
                exprs.insert(exprs.end(), restExprs.begin(), restExprs.end());
                return std::make_shared<ListExpression>(exprs);
            }

            static std::shared_ptr<const Expression> makeFunctionExpression(const std::string& func, const std::vector<std::shared_ptr<const Expression>>& exprs) {
                std::vector<std::shared_ptr<const Expression>> args;
                args.insert(args.end(), exprs.begin(), exprs.end());
                return std::make_shared<FunctionExpression>(func, args);
            }
                   
            static std::shared_ptr<const Expression> makeUnaryExpression(UnaryExpression::Op op, const std::shared_ptr<const Expression>& expr) {
                return std::make_shared<UnaryExpression>(op, expr);
            }

            static std::shared_ptr<const Expression> makeBinaryExpression(BinaryExpression::Op op, const std::shared_ptr<const Expression>& expr1, const std::shared_ptr<const Expression>& expr2) {
                return std::make_shared<BinaryExpression>(op, expr1, expr2);
            }
            
            static std::shared_ptr<const Predicate> makeMapPredicate() {
                return std::make_shared<MapPredicate>();
            }
            
            static std::shared_ptr<const Predicate> makeClassPredicate(const std::string& cls) {
                return std::make_shared<ClassPredicate>(cls);
            }

            static std::shared_ptr<const Predicate> makeLayerPredicate(const std::string& layer) {
                return std::make_shared<LayerPredicate>(layer);
            }
            
            static std::shared_ptr<const Predicate> makeAttachmentPredicate(const std::string& attachment) {
                return std::make_shared<AttachmentPredicate>(attachment);
            }

            static std::shared_ptr<const Predicate> makeOpPredicate(OpPredicate::Op op, bool field, const std::string& fieldOrVar, const Value& value) {
                return std::make_shared<OpPredicate>(op, field, fieldOrVar, value);
            }

            static PropertyDeclaration makePropertyDeclaration(int& order, const std::string& field, const std::shared_ptr<const Expression>& expr) {
                return PropertyDeclaration(field, expr, order++);
            }

            int _declarationOrder = 0;
            std::string::size_type _errorPos = std::string::npos;
        };
    }
    
    StyleSheet CartoCSSParser::parse(const std::string& cartoCSS) {
        std::string::const_iterator it = cartoCSS.begin();
        std::string::const_iterator end = cartoCSS.end();
        cssparserimpl::Grammar<std::string::const_iterator> grammar;
        cssparserimpl::Skipper<std::string::const_iterator> skipper;
        StyleSheet styleSheet;
        bool result = boost::spirit::qi::phrase_parse(it, end, grammar, skipper, styleSheet);
        if (!result) {
            if (grammar.errorPos() != std::string::npos) {
                throw ParserError("Syntax error", resolvePosition(cartoCSS, grammar.errorPos()));
            }
            else {
                throw ParserError("Parsing error");
            }
        }
        else if (it != cartoCSS.end()) {
            throw ParserError("Failed to parse to the end", resolvePosition(cartoCSS, it - cartoCSS.begin()));
        }
        return styleSheet;
    }

    std::pair<int, int> CartoCSSParser::resolvePosition(const std::string& str, std::string::size_type pos) {
        std::pair<int, int> colLine(1, 1);
        for (std::string::size_type i = 0; i < pos; i++) {
            if (str[i] == '\n') {
                colLine.second++;
                colLine.first = 1;
            }
        }
        return colLine;
    }
} }
