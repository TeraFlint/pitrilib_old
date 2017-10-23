#include "filemanip.h"

namespace Pitri
{
	FileSector::FileSector(std::string newname) : name(newname)
	{
	}
	bool FileSector::Good()
	{
		return !name.empty() && (!entries.empty() || !lines.empty());
	}

	std::vector<FileSector> ScanSectorFile(std::string path, char assigner)
	{
		std::ifstream file(path);
		if (file.is_open())
		{
			std::vector<FileSector> result;
			FileSector sector("Main");

			std::string line;
			bool cmt = false;
			while (std::getline(file, line))
			{
				line = TrimString(ParseComment(line, &cmt));
				if (!line.empty())
				{
					if (line[0] == '[')
					{
						unsigned index = line.find(']');
						if (index != std::string::npos && index > 2)
						{
							std::string name = line.substr(1, index - 1);
							if (sector.Good())
								result.push_back(sector);
							sector = FileSector(TrimString(name));
						}
					}
					else
					{
						unsigned index = line.find(assigner);
						if (index != std::string::npos)
						{
							std::vector<std::string> parts = Explode(line, assigner);
							std::string key = parts[0];
							std::string contents = "";
							for (unsigned i = 1; i < parts.size(); ++i)
							{
								if (!contents.empty())
									contents += assigner;
								contents += parts[i];
							}

							if (!key.empty() && !contents.empty())
								sector.entries[key] = contents;
						}
						else
							sector.lines.push_back(line);
					}
				}
			}
			if (!sector.entries.empty())
				result.push_back(sector);

			file.close();
			return result;
		}
		return{};
	}

	std::map<std::string, FileEntry> ScanSimpleFile(std::string filename, char assigner, char separator)
	{
		std::map<std::string, FileEntry> result;
		std::ifstream file(filename);
		if (!file.is_open()) return result;

		std::string line;
		bool comment = false;
		while (std::getline(file, line))
		{
			//remove comments and trim the result
			line = TrimString(ParseComment(line, &comment));

			//still contains data?
			if (!line.length())
				continue;

			unsigned index = line.find(assigner);
			if (index == std::string::npos)
				continue;

			std::string key = TrimString(line.substr(0, index));
			std::string value = TrimString(line.substr(index + 1, line.length() - (index + 1)));

			FileEntry entry = {};
			entry.key = key;
			entry.contents = Explode(value, separator);
			entry.valid = true;

			if (!key.empty() && !entry.contents.empty())
				result[key] = entry;
		}
		file.close();
		return result;
	}

	std::string GetSimpleFileValue(std::string filename, std::string key, char assigner)
	{
		std::ifstream file(filename);
		if (file.is_open())
		{
			std::string line;
			bool comment = false;
			while (std::getline(file, line))
			{
				//remove comments and trim the result
				line = TrimString(ParseComment(line, &comment));

				//still contains data?
				if (!line.length())
					continue;

				unsigned index = line.find(assigner);
				if (index == std::string::npos)
					continue;

				std::string keyword = TrimString(line.substr(0, index));
				std::string value = TrimString(line.substr(index + 1, line.length() - (index + 1)));

				if (!key.compare(keyword))
				{
					file.close();
					return value;
				}
			}
			file.close();
		}
		return "";
	}

	bool CreateDirectoryPath(std::string path)
	{
		std::string local = "";
		std::vector<std::string> folders = Explode(path, '\\');
		for (auto str : folders)
		{
			if (!local.empty())
				local += "\\";
			local += str;
			if (!CreateDirectory(local.c_str(), 0))
				return false;
		}
		return true;
	}

	std::vector<std::string> ScanDirectory(std::string path, unsigned search, std::vector<std::string> blacklist)
	{
		if (path.empty()) path = ".";
		DIR *dir = opendir(path.c_str());
		if (dir && search)
		{
			std::vector<std::string> result;
			dirent *entry = readdir(dir);
			while (entry)
			{
				bool valid = true;
				for (std::string forbidden : blacklist)
				{
					if (!forbidden.compare(entry->d_name))
					{
						valid = false;
						break;
					}
				}
				if (valid && entry->d_type & search)
					result.push_back(entry->d_name);
				entry = readdir(dir);
			}
			return result;
		}
		return{};
	}

	void DeleteDirectoryContents(std::string path)
	{
		SetStringEnd(path, "\\");

		std::vector<std::string> files = ScanDirectory(path, DT_REG);
		std::vector<std::string> dirs = ScanDirectory(path, DT_DIR);

		for (auto file : files)
		{
			std::string filename = path + file;
			DeleteFile(filename.c_str());
		}
		for (auto dir : dirs)
		{
			std::string dirname = path + dir;
			DeleteDirectoryContents(dirname);
			RemoveDirectory(dirname.c_str());
		}
	}

	void CopyDirectoryContents(std::string from, std::string to, bool no_overwrite)
	{
		SetStringEnd(from, "\\");
		SetStringEnd(to, "\\");

		std::vector<std::string> files = ScanDirectory(from, DT_REG);
		std::vector<std::string> dirs = ScanDirectory(from, DT_DIR);

		CreateDirectoryPath(to);
		for (auto file : files)
		{
			CopyFile((from + "\\" + file).c_str(), (to + "\\" + file).c_str(), no_overwrite);
		}
		for (auto dir : dirs)
		{
			CreateDirectoryPath(to + "\\" + dir);
			CopyDirectoryContents(from + "\\" + dir, to + "\\" + dir);
		}
	}
}