/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_TAGGEDTOKENLIST_H_
#define _CARTO_GEOCODING_TAGGEDTOKENLIST_H_

#include <cstdint>
#include <string>
#include <set>
#include <vector>
#include <utility>
#include <algorithm>

namespace carto { namespace geocoding {
    template <typename StringType, typename TokenType, typename TagType>
    class TaggedTokenList final {
    public:
        using CharType = typename StringType::value_type;

        struct Span {
            int index;
            int count;

            explicit Span(int index, int count) : index(index), count(count) { }
        };
        
        TaggedTokenList() = default;

        int size() const { return static_cast<int>(_tokens.size()); }

        int unmatchedTokens() const {
            return static_cast<int>(std::count_if(_tokens.begin(), _tokens.end(), [](const Token& token) { return !token.value.empty() && token.assignedType == TokenType(); }));
        }

        int unmatchedInvalidTokens(std::uint32_t validTypeMask = std::numeric_limits<std::uint32_t>::max()) const {
            return static_cast<int>(std::count_if(_tokens.begin(), _tokens.end(), [validTypeMask](const Token& token) { return !token.value.empty() && token.assignedType == TokenType() && (token.validTypeMask & validTypeMask) == 0; }));
        }

        void setTag(int index, const TagType& tag) {
            _tokens.at(index).tag = tag;
        }

        void setIDF(int index, float idf) {
            _tokens.at(index).idf = idf;
        }

        void setValidTypeMask(int index, std::uint32_t validTypeMask) {
            _tokens.at(index).validTypeMask = validTypeMask;
        }

        void assignType(const Span& span, TokenType type) {
            std::uint32_t typeMask = static_cast<std::uint32_t>(1) << static_cast<int>(type);
            for (std::size_t i = span.index; i < span.index + span.count; i++) {
                _tokens.at(i).assignedType = type;
                _tokens.at(i).validTypeMask = 0;
            }
            for (std::size_t i = 0; i < _tokens.size(); i++) {
                _tokens.at(i).validTypeMask &= ~typeMask;
            }
        }

        TokenType prevType(const Span& span) const {
            if (span.index < 0) {
                return TokenType();
            }

            for (std::size_t i = span.index; i > 0; i--) {
                if (!_tokens[i - 1].value.empty()) {
                    return _tokens[i - 1].assignedType;
                }
            }
            return TokenType();
        }

        TokenType nextType(const Span& span) const {
            if (span.index < 0) {
                return TokenType();
            }

            for (std::size_t i = span.index + span.count; i < _tokens.size(); i++) {
                if (!_tokens[i].value.empty()) {
                    return _tokens[i].assignedType;
                }
            }
            return TokenType();
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

        bool valid() const {
            if (std::count_if(_tokens.begin(), _tokens.end(), [](const Token& token) {
                return !token.value.empty() && token.assignedType == TokenType() && token.validTypeMask == 0;
            }) != 0) {
                return false;
            }

            std::uint32_t validTypeMaskUnion = 0;
            for (std::size_t i = 0; i < _tokens.size(); i++) {
                validTypeMaskUnion |= _tokens[i].validTypeMask;
            }

            for (std::uint32_t typeMask = 1; typeMask <= validTypeMaskUnion; typeMask <<= 1) {
                std::size_t minIndex = _tokens.size(), maxIndex = 0;
                for (std::size_t i = 0; i < _tokens.size(); i++) {
                    if (_tokens[i].validTypeMask == typeMask) {
                        minIndex = std::min(minIndex, i);
                        maxIndex = std::max(maxIndex, i);
                    }
                }
                for (std::size_t i = minIndex; i <= maxIndex; i++) {
                    if (!(_tokens[i].validTypeMask & typeMask)) {
                        return false;
                    }
                }
            }
            return true;
        }

        void enumerateSpans(std::uint32_t validTypeMask, std::uint32_t strictTypeMask, TokenType& type, std::vector<Span>& spans) const {
            type = TokenType();
            float maxIdf = -std::numeric_limits<float>::infinity();
            for (std::size_t k = 0; k < _tokens.size(); k++) {
                if (_tokens[k].idf > maxIdf) {
                    std::uint32_t typeMask = _tokens[k].validTypeMask & validTypeMask;
                    if (typeMask != 0) {
                        for (int i = 0; i < 32; i++) {
                            if ((typeMask & (static_cast<std::uint32_t>(1) << i)) != 0) {
                                type = static_cast<TokenType>(i);
                                maxIdf = _tokens[k].idf;
                                break;
                            }
                        }
                    }
                }
            }
            if (type == TokenType()) {
                return;
            }

            std::uint32_t typeMask = static_cast<std::uint32_t>(1) << static_cast<int>(type);
            for (std::size_t j = _tokens.size(); j > 0; j--) {
                for (std::size_t i = 0; i < j; i++) {
                    bool valid = true;
                    for (std::size_t k = 0; k < _tokens.size(); k++) {
                        if (k >= i && k < j) {
                            if (_tokens[k].assignedType != TokenType() || !(_tokens[k].validTypeMask & typeMask)) {
                                valid = false;
                            }
                        }
                        else {
                            if (_tokens[k].validTypeMask == typeMask) {
                                valid = false;
                            }
                        }
                        if (!valid) {
                            break;
                        }
                    }
                    if (valid) {
                        Span span(static_cast<int>(i), static_cast<int>(j - i));
                        if ((typeMask & strictTypeMask) == 0) {
                            for (std::size_t k = 0; k < spans.size(); k++) {
                                if (spans[k].count == span.count) {
                                    bool match = true;
                                    for (int offset = 0; offset < span.count; offset++) {
                                        if (_tokens[span.index + offset].value != _tokens[spans[k].index + offset].value) {
                                            match = false;
                                            break;
                                        }
                                    }
                                    if (match) {
                                        valid = false;
                                        break;
                                    }
                                }
                            }
                        }
                        if (valid) {
                            spans.push_back(span);
                        }
                    }
                }
            }

            std::sort(spans.begin(), spans.end(), [this](const Span& span1, const Span& span2) {
                if (span1.count == span2.count) {
                    int count1 = 0, count2 = 0;
                    for (int offset = 0; offset < span1.count; offset++) {
                        count1 += bitCount(_tokens[span1.index + offset].validTypeMask);
                        count2 += bitCount(_tokens[span2.index + offset].validTypeMask);
                    }
                    return count1 < count2;
                }
                return span1.count > span2.count;
            });
            
            bool skip = false;
            for (std::size_t k = 0; k < _tokens.size(); k++) {
                if (_tokens[k].validTypeMask == typeMask) {
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                spans.emplace_back(Span(-1, 0));
            }
        }

        static TaggedTokenList build(const StringType& text) {
            std::vector<Token> tokens;
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
            return TaggedTokenList(tokens);
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
            std::uint32_t validTypeMask = 0;
            float idf = 1.0f;

            explicit Token(StringType value) : value(std::move(value)) { }
        };

        explicit TaggedTokenList(std::vector<Token> tokens) : _tokens(std::move(tokens)) { }

        static int bitCount(std::uint32_t value) {
            int count = 0;
            while (value) {
                count += value & 1;
                value >>= 1;
            }
            return count;
        }

        std::vector<Token> _tokens;
    };
} }

#endif
