#include "stringmanip.h"

namespace Pitri
{
	std::string ParseComment(std::string input)
	{
		bool buffer = 0;
		return ParseComment(input, &buffer);
	}
	std::string ParseComment(std::string input, bool *disabled)
	{
		std::string skey = "//", mkey1 = "/*", mkey2 = "*/";
		if (!(*disabled))
		{
			unsigned single = input.find(skey), multi = input.find(mkey1);
			if (single == std::string::npos && multi == std::string::npos)
				return input;
			if (single < multi)
				return input.substr(0, single);
			unsigned end = input.find(mkey2, multi + mkey1.size());
			if (end == std::string::npos)
			{
				*disabled = true;
				return input.substr(0, multi);
			}
			std::string before = input.substr(0, multi), after = input.substr(end + mkey2.size(), input.size() - mkey2.size() - end);
			return before + ParseComment(after, disabled);
		}
		else
		{
			unsigned end = input.find(mkey2);
			if (end == std::string::npos)
				return "";

			*disabled = false;
			std::string after = input.substr(end + mkey2.size(), input.size() - mkey2.size() - end);
			return ParseComment(after, disabled);
		}
	}

	std::string TrimString(std::string input)
	{
		char *list = " \t\r\n";
		unsigned size = sizeof(list) / sizeof(list[0]);

		if (FindExcept(input, list) == std::string::npos)
			return "";

		unsigned start = 0, end = input.length();
		for (unsigned i = start; i < end; ++i)
		{
			bool success = true;
			for (unsigned j = 0; j < size; ++j)
			{
				if (input[i] == list[j])
				{
					success = false;
					break;
				}
			}
			if (success)
			{
				start = i;
				break;
			}
		}
		for (unsigned i = end - 1; i >= start; --i)
		{
			bool success = true;
			for (unsigned j = 0; j < size; ++j)
			{
				if (input[i] == list[j])
				{
					success = false;
					break;
				}
			}
			if (success)
			{
				end = i + 1;
				break;
			}
		}

		if (start >= end) return "";
		return input.substr(start, end - start);
	}

	std::vector<std::string> Explode(std::string input, char separator)
	{
		std::vector<std::string> result;
		std::string buffer = input;
		while (buffer.size())
		{
			unsigned index = buffer.find(separator);
			if (index != std::string::npos)
			{
				result.push_back(buffer.substr(0, index));
				buffer = buffer.substr(index + 1, buffer.size() - index - 1);
			}
			else
			{
				result.push_back(buffer);
				buffer = "";
			}
		}
		return result;
	}

	unsigned FindExcept(std::string input, std::string list, unsigned real_offset)
	{
		for (unsigned i = real_offset; i < input.length(); ++i)
		{
			bool success = true;
			for (unsigned j = 0; j < list.length(); ++j)
			{
				if (input[i] == list[j])
				{
					success = false;
					break;
				}
			}
			if (success)
				return i;
		}
		return std::string::npos;
	}

	void SetStringEnd(std::string &input, std::string end)
	{
		if (input.substr(input.size() - end.size()).compare(end))
			input += end;
	}
}