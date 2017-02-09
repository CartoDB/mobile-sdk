/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_TOKENLIST_H_
#define _CARTO_GEOCODING_TOKENLIST_H_

#include <string>
#include <set>
#include <vector>
#include <utility>
#include <algorithm>

namespace carto { namespace geocoding {
    template <typename StringType, typename TagType>
    class TokenList final {
    public:
        using CharType = typename StringType::value_type;

        struct Span {
            int index;
            int count;
        };
        
        TokenList() = default;

        std::size_t size() const { return _tokens.size(); }

        const std::pair<StringType, int>& at(std::size_t i) const { return _tokens.at(i); }

        StringType name(TagType type) const {
            for (const std::pair<TagType, StringType>& tokenNames : _tokenNames) {
                if (tokenNames.first == type) {
                    return tokenNames.second;
                }
            }
            return StringType();
        }

        std::vector<StringType> tokens(const Span& span) const {
            std::vector<StringType> tokens;
            for (auto it = _tokens.begin(); it != _tokens.end(); it++) {
                if (it->second >= span.index && it->second < span.index + span.count) {
                    tokens.push_back(it->first);
                }
            }
            return tokens;
        }

        TagType type(int index) const {
            for (const std::pair<TagType, Span>& tokenType : _tokenTypes) {
                if (index >= tokenType.second.index && index < tokenType.second.index + tokenType.second.count) {
                    return tokenType.first;
                }
            }
            return TagType();
        }

        Span span(TagType type) const {
            for (const std::pair<TagType, Span>& tokenType : _tokenTypes) {
                if (tokenType.first == type) {
                    return tokenType.second;
                }
            }
            return Span { -1, 0 };
        }

        void mark(const Span& span, TagType type) {
            StringType name;
            for (auto it = _tokens.begin(); it != _tokens.end(); ) {
                if (it->second >= span.index && it->second < span.index + span.count) {
                    if (!name.empty()) {
                        name.append(1, ' ');
                    }
                    name += it->first;
                    it = _tokens.erase(it);
                }
                else {
                    it++;
                }
            }
            _tokenTypes.emplace_back(type, span);
            _tokenNames.emplace_back(type, name);
        }

        std::vector<Span> enumerate() const {
            std::vector<Span> results;
            for (std::size_t i = 0; i < _tokens.size(); i++) {
                for (std::size_t j = i + 1; j <= _tokens.size(); j++) {
                    int count = static_cast<int>(j - i);
                    results.push_back({ _tokens[i].second, count });
                    if (j < _tokens.size()) {
                        if (_tokens[j].second != _tokens[i].second + count) {
                            break;
                        }
                    }
                }
            }
            std::sort(results.begin(), results.end(), [](const Span& span1, const Span& span2) {
                return std::tie(span1.count, span1.index) > std::tie(span2.count, span2.index);
            });
            return results;
        }

        static TokenList build(const StringType& text) {
            std::vector<std::pair<StringType, int>> tokens;
            std::vector<std::pair<TagType, Span>> tokenTypes;
            int count = 0;
            std::size_t i0 = 0, i1 = 0;
            while (i1 < text.size()) {
                CharType c = text[i1];
                if (isSpace(c) || isSeparator(c)) {
                    if (i1 > i0) {
                        tokens.emplace_back(text.substr(i0, i1 - i0), count);
                        count++;
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
                        tokenTypes.emplace_back(TagType(), Span { count, 1 });
                        count++;
                    }
                    i0 = i1;
                }
                else {
                    i1++;
                }
            }
            if (i1 > i0) {
                tokens.emplace_back(text.substr(i0, i1 - i0), count);
            }
            return TokenList { tokens, tokenTypes };
        }

        static bool isSpace(CharType c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        }

        static bool isSeparator(CharType c) {
            return c == ',' || c == ';';
        }
        
    private:
        explicit TokenList(std::vector<std::pair<StringType, int>> tokens, std::vector<std::pair<TagType, Span>> tokenTypes) : _tokens(std::move(tokens)), _tokenTypes(std::move(tokenTypes)) { }

        std::vector<std::pair<StringType, int>> _tokens;
        std::vector<std::pair<TagType, Span>> _tokenTypes;
        std::vector<std::pair<TagType, StringType>> _tokenNames;
    };
} }

#endif
