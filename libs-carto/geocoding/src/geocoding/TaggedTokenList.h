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
            return static_cast<int>(std::count_if(_tokens.begin(), _tokens.end(), [](const Token& token) { return !token.value.empty() && token.assignedTypeMask == 0; }));
        }

        int unmatchedInvalidTokens() const {
            std::uint32_t validTypeMask = std::numeric_limits<std::uint32_t>::max();
            for (std::size_t i = 0; i < _tokens.size(); i++) {
                if (_tokens[i].assignedTypeMask != 0) {
                    if (bitCount(_tokens[i].assignedTypeMask) == 1) {
                        validTypeMask = validTypeMask & ~_tokens[i].assignedTypeMask;
                    }
                }
            }
            
            return static_cast<int>(std::count_if(_tokens.begin(), _tokens.end(), [validTypeMask](const Token& token) { return !token.value.empty() && token.assignedTypeMask == 0 && (token.validTypeMask & validTypeMask) == 0; }));
        }

        std::vector<std::pair<std::uint32_t, StringType>> assignment() const {
            std::vector<std::pair<std::uint32_t, StringType>> types;
            types.reserve(_tokens.size());
            for (const Token& token : _tokens) {
                if (token.assignedTypeMask != 0) {
                    types.emplace_back(token.assignedTypeMask, token.value);
                }
            }
            std::stable_sort(types.begin(), types.end(), [](const std::pair<std::uint32_t, StringType>& t1, const std::pair<std::uint32_t, StringType>& t2) {
                return t1 < t2;
            });
            return types;
        }

        void setTag(int index, const TagType& tag) {
            _tokens.at(index).tag = tag;
        }

        void setIDF(int index, float idf) {
            _tokens.at(index).idf = idf;
        }

        void setValidTypeMask(int index, std::uint32_t typeMask) {
            _tokens.at(index).validTypeMask = typeMask;
        }

        void assignTypeMask(const Span& span, std::uint32_t typeMask) {
            for (std::size_t i = span.index; i < span.index + span.count; i++) {
                _tokens.at(i).assignedTypeMask = _tokens.at(i).validTypeMask & typeMask;
                _tokens.at(i).validTypeMask = 0;
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

        void enumerateSpans(const std::function<bool(std::uint32_t, const Span&)>& callback) const {
            std::uint32_t validTypeMask = std::numeric_limits<std::uint32_t>::max();
            for (std::size_t i = 0; i < _tokens.size(); i++) {
                if (_tokens[i].assignedTypeMask != 0) {
                    if (bitCount(_tokens[i].assignedTypeMask) == 1) {
                        validTypeMask = validTypeMask & ~_tokens[i].assignedTypeMask;
                    }
                }
            }
            
            for (std::size_t size = _tokens.size(); size > 0; size--) {
                std::vector<std::pair<std::uint32_t, Span>> spans;
                for (std::size_t j = _tokens.size(); j >= size; j--) {
                    std::size_t i = j - size;
                    std::uint32_t typeMask = validTypeMask;
                    for (std::size_t k = i; k < j; k++) {
                        if (_tokens[k].assignedTypeMask != 0) {
                            typeMask = 0;
                        }
                        typeMask &= _tokens[k].validTypeMask;
                    }

                    if (typeMask) {
                        Span span(static_cast<int>(i), static_cast<int>(j - i));
                        spans.emplace_back(typeMask, span);
                    }
                }
                
                std::sort(spans.begin(), spans.end(), [](const std::pair<std::uint32_t, Span>& span1, const std::pair<std::uint32_t, Span>& span2) {
                    return bitCount(span1.first) < bitCount(span2.first);
                });
                
                for (const std::pair<std::uint32_t, Span>& span : spans) {
                    if (!callback(span.first, span.second)) {
                        return;
                    }
                }
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
            std::uint32_t assignedTypeMask = 0; // todo: use bool assigned instead
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
