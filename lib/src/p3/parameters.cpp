#include "parameters.h"
#include <string>

#if P3_OS_WINDOWS
	#include <Windows.h>
#endif

namespace Pitri
{
	Parameter::Parameter()
	{
		key = "";
		desc = {};

		required = 0;
		amount = 0;
		used = 0;

		undocumented = 0;
	}

	std::string Parameter::PrintContents(unsigned real_offset)
	{
		if (key.empty() && entries.empty())
			return "";

		std::string result = key + " (" + std::to_string(entries.size()) + "/" + std::to_string(amount) + ")";
		for (unsigned i = result.length(); i < real_offset; ++i)
			result += " ";
		result += " = ";
		if (!amount)
		{
			result += used ? "true" : "false";
		}
		else if (used)
		{
			if (amount == 1)
			{
				result += "\"" + entries[0] + "\"";
			}
			else
			{
				result += "{ ";
				for (unsigned i = 0; i < entries.size(); ++i)
				{
					if (i) result += ", ";
					result += "\"" + entries[i] + "\"";
				}
				result += " }";
			}
		}
		else
		{
			result += "false";
		}
		return result;
	}

	std::string Parameter::PrintHelp(unsigned real_offset, bool debug)
	{
		std::string result = " -" + key;
		for (unsigned i = result.length(); i < real_offset; ++i)
			result += " ";
		for (unsigned i = 0; i < desc.size(); ++i)
		{
			if (i)
				result += "\n" + std::string(real_offset, ' ');
			result += desc[i];
		}

		std::vector<std::string> properties;

		switch (amount)
		{
		case 0:
		properties.push_back("is a flag");
		break;
		case 1:
		properties.push_back("takes 1 argument");
		break;
		default:
		properties.push_back("takes " + std::to_string(amount) + " arguments");
		}

		if (!required)
			properties.push_back("is optional");

		if (debug && undocumented)
			properties.push_back("is undocumented");

		if (!properties.empty())
		{
			result += "\n" + std::string(real_offset, ' ') + "This parameter ";
			for (unsigned i = 0; i < properties.size(); ++i)
			{
				if (properties.size() > 1)
				{
					if (i == properties.size() - 1)
						result += " and ";
					else if (i)
						result += ", ";
				}
				result += properties[i];
			}
			result += ".";
		}

		return result;
	}

	bool Parameter::AddEntry(std::string entry)
	{
		if (Full())
			return false;
		entries.push_back(entry);
		return true;
	}

	bool Parameter::Full()
	{
		return entries.size() >= amount;
	}

	bool Parameter::ReturnValue(bool &out) const
	{
		if (!used) return false;
		out = true;
		return true;
	}
	bool Parameter::ReturnValue(int &out) const
	{
		std::string result;
		if (!ReturnValue(result))
			return false;

		bool numeric = true;
		for (auto c : result)
		{
			if ((c < '0' || c > '9') && c != '-')
			{
				numeric = false;
				break;
			}
		}
		if (numeric) out = std::stoi(result);
		return true;
	}
	bool Parameter::ReturnValue(unsigned &out) const
	{
		std::string result;
		if (!ReturnValue(result))
			return false;

		bool numeric = true;
		for (auto c : result)
		{
			if (c < '0' || c > '9')
			{
				numeric = false;
				break;
			}
		}
		if (numeric) out = std::stoi(result);
		return true;
	}
	bool Parameter::ReturnValue(float &out) const
	{
		double result;
		if (!ReturnValue(result))
			return false;

		out = result;
		return true;
	}
	bool Parameter::ReturnValue(double &out) const
	{
		std::string result;
		if (!ReturnValue(result))
			return false;

		bool numeric = true;
		for (auto c : result)
		{
			if ((c < '0' || c > '9') && c != '-' && c != '.' && c != 'e')
			{
				numeric = false;
				break;
			}
		}
		if (numeric) out = std::stod(result);
		return true;
	}
	bool Parameter::ReturnValue(std::string &out) const
	{
		if (!entries.size() || amount != 1) return false;
		out = entries[0];
		return true;
	}
	bool Parameter::ReturnValue(std::vector<std::string> &out) const
	{
		if (!entries.size()) return false;
		out = entries;
		return true;
	}

	ParList::ParList()
	{
		desc2 = 0;
		initialized = false;
		AddParameter("?", false, 0, "Parameter help and program information. You're looking at it right now.\nEither accessible by -? or by misusage of parameters (depending on the program itself).");
		AddParameter("?!", false, 0, "Same as -?, but also shows undocumented/debug parameters.", true);
	}

#if P3_OS_WINDOWS
	bool ParList::HandleParameters()
	{
		return HandleParameters(GetCommandLine());
	}
#endif

	bool ParList::HandleParameters(int argc, char **argv)
	{
		std::vector<std::string> data;
		for (unsigned i = 0; i < argc; ++i)
		{
			data.push_back(argv[i]);
		}
		return HandleParameters(data);
	}

	/*
	for (auto c : input)
	{
		if (c == '"' && !escape)
			in_string ^= true;
		if (c == '\\') escape ^= true;
		else escape = false;

		if (in_string || c != ' ' && c != '\t' && c != '\r' && c != '\n')
			result += c;
	*/

	bool ParList::HandleParameters(std::string line)
	{
		std::vector<std::string> data;
		bool quote = false;
		bool escape = false;
		std::string buffer = "";

		for (auto c : line)
		{
			if (c == '"' && !escape)
				quote ^= true;
			if (c == '\\') escape ^= true;
			else escape = false;

			if (quote || c != ' ')
			{
				buffer += c;
			}
			else
			{
				data.push_back(buffer);
				buffer = "";
			}
		}

		if (!buffer.empty())
			data.push_back(buffer);
		return HandleParameters(data);
	}

	bool ParList::HandleParameters(std::vector<std::string> data)
	{
		//no data? what are you even doing? get out!
		if (data.empty()) return false;

		//reset all the output values, just in case someone recycles an old parlist
		for (auto &par : list)
		{
			par.second.used = 0;
			par.second.entries = {};
		}

		for (auto &entry : data)
		{
			if (entry.empty())
				continue;

			//unstringify the string.
			if (entry.size() > 1 && entry[0] == '"' && entry[entry.size() - 1] == '"')
			{
				/*
				std::string newstring;
				for (unsigned i = 1; i < entry.size() - 1; ++i)
				{
					if (entry[i] == '\\')
					{
						if (i + 1 < entry.size())
							newstring += entry[i + 1];
						++i;
					}
					else
						newstring += entry[i];
				}
				entry = newstring;*/
				entry = entry.substr(1, entry.size() - 2);
			}
		}

		if (!data[0].empty())
		{
			unsigned index = data[0].rfind('\\');
			if (index != std::string::npos)
			{
				dir = data[0].substr(0, index);
				file = data[0].substr(index + 1, data[0].size() - index - 1);
			}
			else
			{
				dir = "";
				file = data[0];
			}
		}

		if (data.size() < 2)
			return true;

		std::string lastkey = "";

		//compile all the parameters
		for (unsigned i = 1; i < data.size(); ++i)
		{
			if (!data[i].empty())
			{
				bool data_par = true;
				//key parameter?
				if (data[i][0] == '-' || data[i][0] == '/')
				{
					std::string key = data[i].substr(1);
					//valid? begin a new sector
					if (KeyExists(key))
					{
						lastkey = key;
						data_par = false;
						list[key].used = 1;
					}
				}
				//data parameter?
				if (data_par)
				{
					if (lastkey.empty())
					{
						unhandled.push_back(data[i]);
					}
					else
					{
						list[lastkey].entries.push_back(data[i]);
					}
				}
			}

			//reset sector, if large enough
			if (!lastkey.empty() && KeyExists(lastkey) && list[lastkey].Full())
			{
				list[lastkey].used = 2;
				lastkey = "";
			}
		}
		initialized = true;
		return true;
	}

	bool ParList::AddParameter(std::string key, bool required, unsigned amount, std::string desc, bool undocumented)
	{
		std::string buffer = desc;
		std::vector<std::string> description;
		while (buffer.size())
		{
			unsigned index = buffer.find('\n');
			if (index != std::string::npos)
			{
				description.push_back(buffer.substr(0, index));
				buffer = buffer.substr(index + 1, buffer.size() - index - 1);
			}
			else
			{
				description.push_back(buffer);
				buffer = "";
			}
		}
		return AddParameter(key, required, amount, description, undocumented);
	}

	bool ParList::AddParameter(std::string key, bool required, unsigned amount, std::vector<std::string> desc, bool undocumented)
	{
		Parameter par;
		par.key = key;
		par.desc = desc;

		par.required = required;
		par.amount = amount;
		par.used = 0;

		par.undocumented = undocumented;

		return AddParameter(par);
	}

	bool ParList::AddParameter(Parameter par)
	{
		if (par.key.empty())
			return false;

		//it doesn't make sense for a single flag to be required. Flags are for optional stuff.
		if (!par.amount && par.required)
			par.required = false;

		list[par.key] = par;
		keys[par.key] = true;
		return true;
	}

	bool ParList::SetDescription(std::string(*func)())
	{
		desc1 = func;
		desc2 = 0;
		return true;
	}

	bool ParList::SetDescription(std::string(*func)(void *), void* par)
	{
		desc1 = 0;
		desc2 = func;
		desc_par = par;
		return true;
	}

	void ParList::SetExeName(std::string name)
	{
		data.name = name;
	}
	void ParList::SetExeVersion(std::string version)
	{
		data.version = version;
	}
	void ParList::SetExeAuthor(std::string author)
	{
		data.author = author;
	}
	void ParList::SetExeBuild(unsigned build)
	{
		data.build = build;
	}
	void ParList::SetExeInfo(std::string author, std::string name, std::string version, unsigned build)
	{
		SetExeAuthor(author);
		SetExeName(name);
		SetExeVersion(version);
		SetExeBuild(build);
	}

	void ParList::SetExeTime(std::string date, std::string time)
	{
		const std::vector<std::string> months = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
		std::string year = date.substr(7, 4);
		std::string month = date.substr(0, 3);
		std::string day = date.substr(4, 2);

		if (day[0] == ' ')
			day[0] = '0';

		for (unsigned i = 0; i < months.size(); ++i)
		{
			if (!month.compare(months[i]))
			{
				month = std::to_string(i + 1);
				if (month.size() < 2)
					month = "0" + month;
				break;
			}
		}
		data.timestamp = year + "-" + month + "-" + day + ", " + time;
	}

	std::map<std::string, Parameter> ParList::GetParList() const
	{
		return list;
	}

	Parameter ParList::GetParameter(std::string key)
	{
		if (!KeyExists(key))
			return Parameter();
		return list[key];
	}

	bool ParList::KeyExists(std::string key) const
	{
		return keys.find(key) != keys.end();
	}

	unsigned ParList::MaxParLength() const
	{
		unsigned max = 0;
		for (auto p : list)
		{
			if (p.second.key.size() > max)
			{
				max = p.second.key.size();
			}
		}
		return max;
	}

	std::string ParList::GetFile() const
	{
		if (!initialized)
			return "";
		return file;
	}

	std::string ParList::GetDir() const
	{
		if (!initialized)
			return "";
		return dir;
	}

	std::string ParList::GetPath() const
	{
		if (!initialized)
			return "";
		return dir + "\\" + file;
	}

	bool ParList::ReturnValue(std::string key, bool &out) const
	{
		if (!KeyExists(key)) return false;
		auto entry = list.find(key);
		return entry->second.ReturnValue(out);
	}
	bool ParList::ReturnValue(std::string key, int &out) const
	{
		if (!KeyExists(key)) return false;
		auto entry = list.find(key);
		return entry->second.ReturnValue(out);
	}
	bool ParList::ReturnValue(std::string key, unsigned &out) const
	{
		if (!KeyExists(key)) return false;
		auto entry = list.find(key);
		return entry->second.ReturnValue(out);
	}
	bool ParList::ReturnValue(std::string key, float &out) const
	{
		if (!KeyExists(key)) return false;
		auto entry = list.find(key);
		return entry->second.ReturnValue(out);
	}
	bool ParList::ReturnValue(std::string key, double &out) const
	{
		if (!KeyExists(key)) return false;
		auto entry = list.find(key);
		return entry->second.ReturnValue(out);
	}
	bool ParList::ReturnValue(std::string key, std::string &out) const
	{
		if (!KeyExists(key)) return false;
		auto entry = list.find(key);
		return entry->second.ReturnValue(out);
	}
	bool ParList::ReturnValue(std::string key, std::vector<std::string> &out) const
	{
		if (!KeyExists(key)) return false;
		auto entry = list.find(key);
		return entry->second.ReturnValue(out);
	}

	std::string ParList::PrintContents()
	{
		std::string result = "\"" + dir + "\" \"" + file + "\"";
		unsigned length = MaxParLength();
		for (auto p : list)
		{
			result += "\n" + p.second.PrintContents(length + 6);
		}

		if (!unhandled.empty())
		{
			result += "\n\nUnhandled strings:\n";
			for (unsigned i = 0; i < unhandled.size(); ++i)
			{
				if (i) result += ", ";
				result += "\"" + unhandled[i] + "\"";
			}
		}
		return result;
	}

	bool ParList::HasTimestamp() const
	{
		return !data.timestamp.empty();
	}
	std::string ParList::GetTimestamp() const
	{
		return data.timestamp;
	}

	std::string ParList::PrintHelp(bool debug)
	{
		std::string details;
		{
			details = data.name;
			if (!data.version.empty())
			{
				if (details.empty())
					details = "Version " + std::string(data.version);
				else
					details += " v" + std::string(data.version);

				if (data.build)
					details += " [" + std::to_string(data.build) + "]";
			}
			if (!data.author.empty())
			{
				if (details.empty())
					details = "By " + std::string(data.author);
				else
					details += " by " + std::string(data.author);
			}
		}

		std::string result = "=== Program details for \"" + file + "\" ===\n";
		if (!details.empty())
			result += details + ".\n";

		if (desc1)
		{
			std::string description = desc1();
			if (!description.empty())
				result += description;
		}
		else if (desc2)
		{
			std::string description = desc2(desc_par);
			if (!description.empty())
				result += description;
		}

		result += "\n\n=== Parameters ===\nThere are two kinds of parameters: Key parameters and data parameters. Key parameters begin with a '-' or a '/'.";
		result += "\nThe general usage is -key data data ... data, depending on the amount of arguments the parameter takes.";
		result += "\nThe key parameters can occur in any order and parameters without data are called flags.";
		result += "\nFor convenience, the parameters in this list are sorted alphabetically:";

		unsigned length = MaxParLength();
		for (auto p : list)
		{
			if (!p.second.undocumented || debug)
				result += "\n\n" + p.second.PrintHelp(length + 4, debug);
		}

		if (HasTimestamp())
			result += "\n\nCompilation: " + GetTimestamp() + ".";
		
		result += "\n";
		return result;
	}
}