#pragma once

#include "platform.h"
#include <vector>
#include <map>

#if defined(P3_OS) && P3_OS_WINDOWS
	#define ReadParameters(par, stream) par.HandleParameters(); if (par.GetParameter("?").used || par.GetParameter("?!").used) { stream << par.PrintHelp(par.GetParameter("?!").used); return 0xF00; }
#endif

namespace Pitri
{
	struct ExeData
	{
		std::string name;
		std::string version;
		std::string author;
		unsigned build;

		std::string timestamp;
	};

	class Parameter
	{
	public:
		Parameter();

		std::string PrintContents(unsigned real_offset = 0);
		std::string PrintHelp(unsigned real_offset = 0, bool debug = false);

		bool AddEntry(std::string entry);
		bool Full();

		bool ReturnValue(bool &out) const;
		bool ReturnValue(int &out) const;
		bool ReturnValue(unsigned &out) const;
		bool ReturnValue(float &out) const;
		bool ReturnValue(double &out) const;
		bool ReturnValue(std::string &out) const;
		bool ReturnValue(std::vector<std::string> &out) const;

		std::string key; //in
		std::vector<std::string> desc; //in

		/*OUTPUT VALUE: */
		std::vector<std::string> entries; //out

		/*INPUT VALUE: Specifies the amount of data parameters it takes.*/
		unsigned amount;

		/*INPUT VALUE: Should be set, if the parameter is not optional.*/
		bool required;
		/*OUTPUT VALUE: Flag will be set to 1, if the parameter has been used and to 2, if the parameter received enough data.*/
		unsigned char used;

		/*INPUT VALUE: If for whatever reason you should decide that you don't want the parameter to show up in the help menu. Please only do it if it's very important.*/
		bool undocumented;
	};

	class ParList
	{
	private:
		bool initialized;

		std::string dir;
		std::string file;
		std::vector<std::string> unhandled;
		std::map<std::string, Parameter> list;
		std::map<std::string, bool> keys;

		std::string(*desc1)();
		std::string(*desc2)(void *);
		void *desc_par;

		ExeData data;

	public:
		ParList();

#if P3_OS_WINDOWS
		/*HandleParameters() takes the parameters from GetCommandLine() and parses them.*/
		bool HandleParameters();
#endif

		/*HandleParameters() takes the usual c parameters from main() and parses them.
		- argc: Amount of the parameters.
		- argv: Array of c strings, length is argc. That's the data.*/
		bool HandleParameters(int argc, char **argv);

		/*HandleParameters() takes one single string and parses it.
		- line: The command line string.*/
		bool HandleParameters(std::string line);

		/*HandleParameters() takes a vector of strings and parses them.
		This is the core function, all the other functions are simply changing the data into a string vector to call this function.
		- data: The string vector. */
		bool HandleParameters(std::vector<std::string> data);

		bool AddParameter(std::string key, bool required, unsigned amount, std::string desc, bool undocumented = 0);
		bool AddParameter(std::string key, bool required, unsigned amount, std::vector<std::string> desc, bool undocumented = 0);
		bool AddParameter(Parameter par);
		bool SetDescription(std::string(*func)());
		bool SetDescription(std::string(*func)(void *), void* par = 0);

		void SetExeName(std::string name);
		void SetExeVersion(std::string version);
		void SetExeAuthor(std::string author);
		void SetExeBuild(unsigned build);
		void SetExeInfo(std::string author, std::string name, std::string version = "", unsigned build = 0);

		void SetExeTime(std::string date, std::string time);

		std::map<std::string, Parameter> GetParList() const;
		Parameter GetParameter(std::string key);
		bool KeyExists(std::string key) const;
		unsigned MaxParLength() const;

		std::string GetFile() const;
		std::string GetDir() const;
		std::string GetPath() const;

		bool ReturnValue(std::string key, bool &out) const;
		bool ReturnValue(std::string key, int &out) const;
		bool ReturnValue(std::string key, unsigned &out) const;
		bool ReturnValue(std::string key, float &out) const;
		bool ReturnValue(std::string key, double &out) const;
		bool ReturnValue(std::string key, std::string &out) const;
		bool ReturnValue(std::string key, std::vector<std::string> &out) const;

		bool HasTimestamp() const;
		std::string GetTimestamp() const;

		std::string PrintContents();
		std::string PrintHelp(bool debug = false);
	};
}