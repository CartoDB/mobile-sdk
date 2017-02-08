/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_STRINGMATCHER_H_
#define _CARTO_GEOCODING_STRINGMATCHER_H_

#include <vector>
#include <utility>
#include <algorithm>
#include <functional>
#include <unordered_map>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace carto { namespace geocoding {
    template <typename StringType>
    class StringMatcher final {
    public:
        using CharType = typename StringType::value_type;

        explicit StringMatcher(std::function<float(const StringType&)> idf) : _idf(std::move(idf)) { }

        void setMaxDist(int maxDist) { _maxDist = maxDist; }
        void setWildcardChar(CharType wildcardChar, float cost) { _wildcardChar = wildcardChar; _wildcardCost = cost; }
        void setTranslationTable(std::unordered_map<CharType, StringType> table, float cost) { _translationTable = std::move(table); _translationCost = cost; }

        float calculateRating(const StringType& queryStr, const StringType& candidatesStr) const {
            WordVector query = splitString(queryStr);
            WordVector candidates = splitString(candidatesStr);
            return rating(query, candidates);
        }

    private:
        struct Word {
            StringType value;
            bool containsWildcard = false;
            bool needsTranslation = false;
        };

        using WordVector = std::vector<Word>;
        using AlignmentVector = std::vector<std::pair<std::size_t, std::size_t>>;

        float levenshtein(const Word& word1, const Word& word2) const {
            const StringType& s1 = word1.value;
            const StringType& s2 = word2.value;

            std::vector<float> distances(s1.size() * s2.size());

            auto setDistance = [&](int i1, int i2, float dist) {
                distances[i1 * s2.size() + i2] = dist;
            };
            auto getDistance = [&](int i1, int i2) -> float {
                if (i1 < 0) {
                    return static_cast<float>(i2 + 1);
                }
                else if (i2 < 0) {
                    return static_cast<float>(i1 + 1);
                }
                return distances[i1 * s2.size() + i2];
            };

            for (int i2 = 0; i2 < static_cast<int>(s2.size()); i2++) {
                for (int i1 = 0; i1 < static_cast<int>(s1.size()); i1++) {
                    float dist = getDistance(i1 - 1, i2 - 1);
                    
                    if (s1[i1] != s2[i2]) {
                        if (s1[i1] == _wildcardChar) {
                            dist = std::min({ _wildcardCost + dist, _wildcardCost + getDistance(i1, i2 - 1), getDistance(i1 - 1, i2) });
                        }
                        else {
                            dist = std::min({ 1 + dist, 1 + getDistance(i1, i2 - 1), 1 + getDistance(i1 - 1, i2) });

                            if (word1.needsTranslation) {
                                auto it1 = _translationTable.find(s1[i1]);
                                if (it1 != _translationTable.end()) {
                                    const StringType& t1 = it1->second;
                                    int j2 = i2 + 1 - static_cast<int>(t1.size());
                                    if (j2 >= 0 && s2.substr(j2, t1.size()) == t1) {
                                        dist = _translationCost + getDistance(i1 - 1, j2 - 1);
                                    }
                                }
                            }

                            if (word2.needsTranslation) {
                                auto it2 = _translationTable.find(s2[i2]);
                                if (it2 != _translationTable.end()) {
                                    const StringType& t2 = it2->second;
                                    int j1 = i1 + 1 - static_cast<int>(t2.size());
                                    if (j1 >= 0 && s1.substr(j1, t2.size()) == t2) {
                                        dist = _translationCost + getDistance(j1 - 1, i2 - 1);
                                    }
                                }
                            }
                        }
                    }
                    
                    setDistance(i1, i2, dist);
                }
            }
            return distances.back();
        }

        float clippedDistance(const Word& word1, const Word& word2) const {
            if (!word1.containsWildcard && !word1.needsTranslation && !word2.containsWildcard && !word2.needsTranslation) {
                std::size_t len1 = word1.value.size();
                std::size_t len2 = word2.value.size();
                if (len1 + _maxDist < len2 || len2 + _maxDist < len1) {
                    return static_cast<float>(_maxDist + 1);
                }
            }
            float dist = levenshtein(word1, word2);
            return dist > _maxDist ? static_cast<float>(_maxDist + 1) : dist;
        }

        float similarity(const Word& word1, const Word& word2) const {
            float dist = clippedDistance(word1, word2);
            if (dist > _maxDist) {
                return 0.0f;
            }
            return 1.0f - dist / static_cast<float>(word2.value.empty() ? 1 : word2.value.size());
        }

        float alignWords(const WordVector& words1, const WordVector& words2, AlignmentVector& alignment) const {
            std::vector<float> costTable(words1.size() * words2.size());

            const float skipCost = static_cast<float>(_maxDist + 1);
            auto setCost = [&](std::size_t i1, std::size_t i2, float cost) {
                costTable[i1 * words2.size() + i2] = cost;
            };
            auto getCost = [&](std::size_t i1, std::size_t i2) -> float {
                if (i1 >= words1.size() || i2 >= words2.size()) {
                    if (i1 >= words1.size() && i2 >= words2.size()) {
                        return 0.0f;
                    }
                    else if (i1 >= words1.size()) {
                        return (words2.size() - i2) * skipCost;
                    }
                    else {
                        return (words1.size() - i1) * skipCost;
                    }
                }
                return costTable[i1 * words2.size() + i2];
            };

            for (std::size_t i1 = words1.size(); i1-- > 0; ) {
                for (std::size_t i2 = words2.size(); i2-- > 0; ) {
                    float cost1 = clippedDistance(words1[i1], words2[i2]) + getCost(i1 + 1, i2 + 1);
                    float cost2 = skipCost + getCost(i1 + 1, i2);
                    float cost3 = skipCost + getCost(i1, i2 + 1);
                    setCost(i1, i2, std::min({ cost1, cost2, cost3 }));
                }
            }

            std::size_t i1 = 0, i2 = 0;
            while (i1 < words1.size() && i2 < words2.size()) {
                float diff1 = getCost(i1, i2) - getCost(i1 + 1, i2 + 1);
                float diff2 = getCost(i1, i2) - getCost(i1 + 1, i2);
                float diff3 = getCost(i1, i2) - getCost(i1, i2 + 1);
                if (diff2 == skipCost) {
                    i1++;
                }
                else if (diff3 == skipCost) {
                    i2++;
                }
                else {
                    if (diff1 <= _maxDist) {
                        alignment.emplace_back(i1, i2);
                    }
                    i1++; i2++;
                }
            }
            return costTable.front();
        }

        float ratingQ(const WordVector& query, const WordVector& candidates, const AlignmentVector& alignment) const {
            float totalR = 0.0f;
            float totalW = static_cast<float>(query.size() - alignment.size());
            for (const std::pair<std::size_t, std::size_t>& ij : alignment) {
                float w = _idf(candidates[ij.second].value);
                totalR += std::pow(similarity(query[ij.first], candidates[ij.second]), 2) * w;
                totalW += w;
            }
            return totalR / totalW;
        }

        float ratingC(const WordVector& query, const WordVector& candidates, const AlignmentVector& alignments) const {
            float totalR = 0.0f;
            float totalW = 0.0f;
            for (const std::pair<std::size_t, std::size_t>& ij : alignments) {
                totalR += _idf(candidates[ij.second].value);
            }
            for (std::size_t i = 0; i < candidates.size(); i++) {
                totalW += _idf(candidates[i].value);
            }
            return totalR / totalW;
        }

        float rating(const WordVector& query, const WordVector& candidates) const {
            AlignmentVector alignment;
            alignWords(query, candidates, alignment);
            return Q_RATING_WEIGHT * ratingQ(query, candidates, alignment) + (1.0f - Q_RATING_WEIGHT) * ratingC(query, candidates, alignment);
        }

        WordVector splitString(StringType str) const {
            static const CharType whitespace[] = { (CharType) '\n', (CharType) '\t', (CharType) ' ', (CharType) 0 };
            
            std::vector<StringType> tokens;
            boost::trim_if(str, boost::is_any_of(whitespace));
            boost::split(tokens, str, boost::is_any_of(whitespace), boost::token_compress_on);

            WordVector words;
            words.reserve(tokens.size());
            for (const StringType& token : tokens) {
                Word word;
                word.value = token;
                word.containsWildcard = std::any_of(token.begin(), token.end(), [this](CharType c) { return c == _wildcardChar; });
                word.needsTranslation = std::any_of(token.begin(), token.end(), [this](CharType c) { return _translationTable.count(c) > 0; });
                words.push_back(std::move(word));
            }
            return words;
        }

        static constexpr float Q_RATING_WEIGHT = 0.75f;

        int _maxDist = std::numeric_limits<int>::max();
        CharType _wildcardChar = 0;
        float _wildcardCost = 1.0f;
        std::unordered_map<CharType, StringType> _translationTable;
        float _translationCost = 0.0f;
        const std::function<float(const StringType&)> _idf;
    };
} }

#endif
