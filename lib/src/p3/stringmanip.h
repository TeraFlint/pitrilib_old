#pragma once
#include <string>
#include <vector>

namespace Pitri
{
	typedef std::string(*strfunc)(std::string);
	typedef void(*strmanip)(std::string*);

	/*ParseComment() takes a single string, removes all the areas marked by C comment notation and returns the new string.
	Used for one single line, starting in enabled mode.
	- input: Input string to parse.*/
	std::string ParseComment(std::string input);

	/*ParseComment() takes a single string, removes all the areas marked by C comment notation and returns the new string.
	Usable for multiple lines, by using the same bool pointer over and over again.
	- input: Input string to parse.
	- disabled: Pointer to a bool. If true, the line already starts witha multi line comment.*/
	std::string ParseComment(std::string input, bool *disabled);

	/*TrimString() removes "empty" space from start and end of a string. This includes the caracters space, tab and newline.
	- input: Input string to parse.*/
	std::string TrimString(std::string input);

	/*Explode() takes a string and creates segments based on the placement of separator characters.
	The result is a string vector and the separator characters won't show up in the substrings.
	- input: Input string to parse.
	- separator: Character at which positions where the string will be cut.*/
	std::vector<std::string> Explode(std::string input, char separator);

	/*FindExcept() takes a string and returns the index of the first character that is not contained in the list.
	If the function doesn't find a char, std::string::npos will be returned.
	- input: Input string to parse.
	- list: Character blacklist.
	- offset: Start of the search, default is 0.*/
	unsigned FindExcept(std::string input, std::string list, unsigned real_offset = 0);

	/*SetStringEnd() guarantees that a string will and the way you want.
	Can be used for filenames to make sure they have their extension, for instance.
	- input: Reference to the string.
	- end: Enforced end/extension.*/
	void SetStringEnd(std::string &input, std::string end);
}