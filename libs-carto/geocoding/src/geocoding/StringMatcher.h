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

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace carto { namespace geocoding {
	template <typename T>
	class StringMatcher {
	public:
		explicit StringMatcher(std::function<float(const T&)> idf) : _idf(std::move(idf)) { }

		void setMaxDist(int maxDist) { _maxDist = maxDist; }
		void setPercentageCost(float cost) { _percentageCost = cost; }

		float calculateRating(const T& queryStr, const T& candidatesStr) const {
			WordVector query = splitString(queryStr);
			WordVector candidates = splitString(candidatesStr);
			return rating(query, candidates);
		}

	private:
		using CharType = typename T::value_type;
		using WordVector = std::vector<T>;
		using AlignmentVector = std::vector<std::pair<std::size_t, std::size_t>>;

		float levenshtein(const T& s1, const T& s2) const {
			std::vector<float> distances(s1.size() + 1);
			for (std::size_t i = 0; i < distances.size(); i++) {
				distances[i] = static_cast<float>(i);
			}
			for (std::size_t i2 = 0; i2 < s2.size(); i2++) {
				std::vector<float> distances_ = { static_cast<float>(i2 + 1) };
				for (std::size_t i1 = 0; i1 < s1.size(); i1++) {
					if (s1[i1] == s2[i2]) {
						distances_.push_back(distances[i1]);
					}
					else {
						float cost = (s1[i1] == '%' ? _percentageCost : 1.0f);
						distances_.push_back(cost + std::min({ distances[i1], distances[i1 + 1], distances_.back() }));
					}
				}
				std::swap(distances, distances_);
			}
			return distances.back();
		}

		float clippedDistance(const T& s1, const T& s2) const {
			int diff = static_cast<int>(s1.size()) - static_cast<int>(s2.size());
			if (diff < -_maxDist || diff > _maxDist) {
				if (s1.find('%') == T::npos) {
					return static_cast<float>(_maxDist + 1);
				}
			}

			float dist = levenshtein(s1, s2);
			return dist > _maxDist ? static_cast<float>(_maxDist + 1) : dist;
		}

		float similarity(const T& s1, const T& s2) const {
			float dist = clippedDistance(s1, s2);
			if (dist > _maxDist) {
				return 0.0f;
			}
			return 1.0f - dist / static_cast<float>(s2.empty() ? 1 : s2.size());
		}

		float alignWords(const WordVector& words1, const WordVector& words2, AlignmentVector& alignment) const {
			std::vector<float> costTable(words1.size() * words2.size());

			const float skipCost = static_cast<float>(_maxDist + 1);
			auto setCost = [&](std::size_t i1, std::size_t i2, float cost) {
				costTable[i1 * words2.size() + i2] = cost;
			};
			auto getCost = [&](std::size_t i1, std::size_t i2) -> float {
				if (i1 >= words1.size() || i2 >= words2.size()) {
					return i1 >= words1.size() && i2 >= words2.size() ? 0.0f : skipCost;
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
				float w = _idf(candidates[ij.second]);
				totalR += std::pow(similarity(query[ij.first], candidates[ij.second]), 2) * w;
				totalW += w;
			}
			return totalR / totalW;
		}

		float ratingC(const WordVector& query, const WordVector& candidates, const AlignmentVector& alignments) const {
			float totalR = 0.0f;
			float totalW = 0.0f;
			for (const std::pair<std::size_t, std::size_t>& ij : alignments) {
				totalR += _idf(candidates[ij.second]);
			}
			for (std::size_t i = 0; i < candidates.size(); i++) {
				totalW += _idf(candidates[i]);
			}
			return totalR / totalW;
		}

		float rating(const WordVector& query, const WordVector& candidates) const {
			AlignmentVector alignment;
			alignWords(query, candidates, alignment);
			return 0.75f * ratingQ(query, candidates, alignment) + 0.25f * ratingC(query, candidates, alignment);
		}

		static WordVector splitString(T str) {
			static const CharType whitespace[] = { (CharType) '\n', (CharType) '\t', (CharType) ' ', (CharType) 0 };
			
			WordVector tokens;
			boost::trim_if(str, boost::is_any_of(whitespace));
			boost::split(tokens, str, boost::is_any_of(whitespace), boost::token_compress_on);
			return tokens;
		}

		int _maxDist = std::numeric_limits<int>::max();
		float _percentageCost = 1.0f;
		const std::function<float(const T&)> _idf;
	};
} }

#endif
