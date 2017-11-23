#pragma once
#include <vector>

namespace Pitri
{
	/*RunLengthEncodeVector() is a RLE encoding function.
	RLE is an easy compression algorithm, which is the most effective with data of low entropy.
	It returns the compressed vector of pairs (length, content).
	- input: Input vector of any kind.*/
	template<typename T> std::vector<std::pair<unsigned, T>> RunLengthEncodeVector(const std::vector<T> &input)
	{
		if (input.empty()) return{};

		std::vector<std::pair<unsigned, T>> result;
		T last = input[0];
		unsigned count = 0;
		for (auto element : input)
		{
			if (element != last)
			{
				result.push_back({ count, last });
				last = element;
				count = 1;
			}
			else ++count;
		}
		result.push_back({ count, last });
		return result;
	}
	/*RunLengthEncodeVector() is a RLE decoding function.
	RLE is an easy compression algorithm, which is the most effective with data of low entropy.
	It returns the uncompressed data vector.
	- input: Input pair vector (length, content).*/
	template<typename T> std::vector<T> RunLengthDecodeVector(const std::vector<std::pair<unsigned, T>> &input)
	{
		if (input.empty()) return{};

		std::vector<T> result;
		for (auto element : input)
		{
			std::vector<T> temp(element.first, element.second);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}
}