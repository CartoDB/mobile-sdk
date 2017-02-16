/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_TOKENLIST_H_
#define _CARTO_GEOCODING_TOKENLIST_H_

#include <cstdint>
#include <string>
#include <set>
#include <vector>
#include <utility>
#include <algorithm>

namespace carto { namespace geocoding {
    template <typename StringType, typename TokenType, typename TagType>
    class TokenList final {
    public:
        using CharType = typename StringType::value_type;

        struct Span {
            int index;
            int count;
        };
        
        TokenList() = default;

        int size() const { return static_cast<int>(_tokens.size()); }

        int unmatchedTokens() const { return static_cast<int>(std::count_if(_tokens.begin(), _tokens.end(), [](const Token& token) { return !token.value.empty() && token.assignedType == TokenType(); })); }

        void setTag(int index, const TagType& tag) {
            _tokens.at(index).tag = tag;
        }

        const TagType& tag(int index) const {
            return _tokens.at(index).tag;
        }

        void markValidType(int index, TokenType type) {
            std::uint64_t tokenMask = static_cast<std::uint64_t>(1) << static_cast<int>(type);
            _tokens.at(index).validTypes |= tokenMask;
        }

        void assignType(const Span& span, TokenType type) {
            std::uint64_t tokenMask = static_cast<std::uint64_t>(1) << static_cast<int>(type);
            for (std::size_t i = span.index; i < span.index + span.count; i++) {
                _tokens.at(i).assignedType = type;
                _tokens.at(i).validTypes = 0;
            }
            for (std::size_t i = 0; i < _tokens.size(); i++) {
                _tokens.at(i).validTypes &= ~tokenMask;
            }
        }

        StringType name(TokenType type) const {
            StringType name;
            for (const Token& token : _tokens) {
                if (token.assignedType == type) {
                    name += (name.empty() ? "" : " ") + token.value;
                }
            }
            return name;
        }

        Span span(TokenType type) const {
            for (std::size_t i = 0; i < _tokens.size(); i++) {
                if (_tokens[i].assignedType == type) {
                    std::size_t j = i + 1;
                    while (j < _tokens.size()) {
                        if (_tokens[j].assignedType != type) {
                            break;
                        }
                        j++;
                    }
                    return Span { static_cast<int>(i), static_cast<int>(j - i) };
                }
            }
            return Span { -1, 0 };
        }

        std::vector<StringType> tokens(const Span& span) const {
            std::vector<StringType> tokens;
            tokens.reserve(span.count);
            for (std::size_t i = span.index; i < span.index + span.count; i++) {
                tokens.push_back(_tokens.at(i).value);
            }
            return tokens;
        }

        std::vector<TagType> tags(const Span& span) const {
            std::vector<TagType> tags;
            tags.reserve(span.count);
            for (std::size_t i = span.index; i < span.index + span.count; i++) {
                tags.push_back(_tokens.at(i).tag);
            }
            return tags;
        }

        std::vector<Span> enumerate(const std::vector<TokenType>& validTypes, TokenType& type) const {
            int bestClassCount = 0, bestMultiClassCount = std::numeric_limits<int>::max();
            for (std::size_t i = 0; i < validTypes.size(); i++) {
                std::uint64_t tokenMask = static_cast<std::uint64_t>(1) << static_cast<int>(validTypes[i]);
                int classCount = 0, multiClassCount = 0;
                for (std::size_t k = 0; k < _tokens.size(); k++) {
                    if (_tokens[k].validTypes & tokenMask) {
                        classCount += 1;
                        for (std::uint64_t validTypes = _tokens[k].validTypes & ~tokenMask; validTypes != 0; validTypes >>= 1) {
                            multiClassCount += validTypes & 1;
                        }
                    }
                }
                if (classCount > 0 && std::make_pair(multiClassCount, -classCount) < std::make_pair(bestMultiClassCount, -bestClassCount)) {
                    type = validTypes[i];
                    bestClassCount = classCount;
                    bestMultiClassCount = multiClassCount;
                }
            }
            if (bestClassCount == 0) {
                type = TokenType();
                return std::vector<Span>();
            }

            std::uint64_t tokenMask = static_cast<std::uint64_t>(1) << static_cast<int>(type);
            std::vector<Span> results;
            for (std::size_t j = _tokens.size(); j > 0; j--) {
                for (std::size_t i = 0; i < j; i++) {
                    bool valid = true;
                    for (std::size_t k = 0; k < _tokens.size(); k++) {
                        if (k >= i && k < j) {
                            if (_tokens[k].assignedType != TokenType() || !(_tokens[k].validTypes & tokenMask)) {
                                valid = false;
                            }
                        }
                        else {
                            if (_tokens[k].validTypes == tokenMask) {
                                valid = false;
                            }
                        }
                    }
                    if (valid) {
                        results.emplace_back(Span { static_cast<int>(i), static_cast<int>(j - i) });
                    }
                }
            }
            
            bool skip = false;
            for (std::size_t k = 0; k < _tokens.size(); k++) {
                if (_tokens[k].validTypes == tokenMask) {
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                results.emplace_back(Span { -1, 0 });
            }
            return results;
        }

        static TokenList build(const StringType& text) {
            std::vector<Token> tokens;
            int count = 0;
            std::size_t i0 = 0, i1 = 0;
            while (i1 < text.size()) {
                CharType c = text[i1];
                if (isSpace(c) || isSeparator(c)) {
                    if (i1 > i0) {
                        tokens.push_back(Token(text.substr(i0, i1 - i0)));
                    }
                    bool stop = false;
                    while (i1 < text.size()) {
                        CharType d = text[i1];
                        if (!(isSpace(d) || isSeparator(d))) {
                            break;
                        }
                        if (isSeparator(d)) {
                            stop = true;
                        }
                        i1++;
                    }
                    if (stop) {
                        tokens.push_back(Token(StringType()));
                    }
                    i0 = i1;
                }
                else {
                    i1++;
                }
            }
            if (i1 > i0) {
                tokens.push_back(Token(text.substr(i0, i1 - i0)));
            }
            return TokenList(tokens);
        }

        static bool isSpace(CharType c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        }

        static bool isSeparator(CharType c) {
            return c == ',' || c == ';';
        }
        
    private:
        struct Token {
            StringType value;
            TagType tag = TagType();
            TokenType assignedType = TokenType();
            std::uint64_t validTypes = 0;

            explicit Token(StringType value) : value(std::move(value)) { }
        };

        explicit TokenList(std::vector<Token> tokens) : _tokens(std::move(tokens)) { }

        std::vector<Token> _tokens;
    };
} }

#endif
