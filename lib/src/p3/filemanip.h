#pragma once
#include "stringmanip.h"
#include <dirent.h>
#include <fstream>
#include <vector>
#include <map>

namespace Pitri
{
	typedef std::map<std::string, std::string> stringmap;

	struct FileEntry
	{
		//Keyword of the entry.
		std::string key;
		//Values of the entry, usually separated by commata in the source file.
		std::vector<std::string> contents;
		//Check this value to make sure that the entry is valid.
		bool valid;
	};

	class FileSector
	{
	public:
		FileSector() = default;
		FileSector(std::string newname);
		bool Good();

		std::string name;
		std::vector<std::string> lines;
		std::map<std::string, std::string> entries;
	};

	/*ScanSectorFile() reads a file of a specific format and returns the values in a nested structure.
	The file can be segmented into sectors, specified by a "[Sector Name]" line. There can be multiple sectors with the same name.
	If a key is used multiple times, it will always be overwritten with the last value. Use multiple values with commata in a single line instead.
	- path: Path of the file.
	- assigner: Character which separates key and contents, default is '='.
	- separator: Character which separates multiple values of an entry, default is ','*/
	std::vector<FileSector> ScanSectorFile(std::string path = ".", char assigner = '=');

	/*ScanSimpleFile() reads a file without sectors, the keys have to be unique, or else it will become the last used value.
	- filename: Path of the file.
	- assigner: Character which separates key and contents, default is '='.
	- separator: Character which separates multiple values of an entry, default is ','*/
	std::map<std::string, FileEntry> ScanSimpleFile(std::string filename, char assigner = '=', char separator = ',');

	/*GetSimpleFileValue() is a fast way to find the value of a key.
	If there are multiple entries (key=value) with the same key, the first one will be returned.
	- filename: Path of the file.
	- key: The key you search for.
	- assigner: Character which separates key and contents, default is '='.*/
	std::string GetSimpleFileValue(std::string filename, std::string key, char assigner = '=');

	/*CreateDirectory() creates the folder at the desired path.
	It goes through the whole path and creates one folder after the other, until it's finished or it can't do it.
	Returns false, if the directory can't be created (no write permissions or path string faulty).
	- path: Complete path.*/
	bool CreateDirectoryPath(std::string path);

	/*ScanDirectory() scans a whole directory and returns all the filenames.
	- path: Path of the directory.
	- search: Bitmask. Use DT_REG for files and DT_DIR for directories.
	- blacklist: If a file directory name equals one of the entries, it won't show up. Default are the "." and ".." entries.*/
	std::vector<std::string> ScanDirectory(std::string path = ".", unsigned search = DT_REG, std::vector<std::string> blacklist = { ".", ".." });

	/*DeleteDirectoryContents() goes through the directory and deletes the contents recursively.
	-path: Path of the directory.*/
	void DeleteDirectoryContents(std::string path);

	/*CopyDirectoryContents() goes through a directory and copies all the contents into another directory.
	- from: Path of the source directory.
	- to: Path of the destinaiton directory.
	- no_overwrite: If true, files will fail to paste, if there already exists a file at the destination. */
	void CopyDirectoryContents(std::string from, std::string to, bool no_overwrite = false);

	std::string RandomTempFolder(std::string label = "", unsigned length = 10);
}