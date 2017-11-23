#pragma once
#include <string>

namespace Pitri
{
	const std::string ZIP_EXT = ".zip";

	/*ZipFile() zips a file. Uses ZLib.
	Returns false, if file and zip are identical, file doesn't exist or zip can't be written. True on success.
	- file: Path to the source file, has to exist.
	- zip: Path to the zip file, gets overwritten.*/
	bool ZipFile(std::string file, std::string zip);
	/*UnzipFile() unzips a file. Uses ZLib.
	Returns false, if zip and file are identical, zip doesn't exist or file can't be written. True on success.
	- zip: Path to the source zip file, has to exist.
	- file: Path to the file, gets overwritten.*/
	bool UnzipFile(std::string zip, std::string file);

	/*ZipFolder() zips an entire folder. Uses ZLib.
	Returns false, if dir and zip are identical, dir doesn't exist, is empty or zip can't be written. True on success.
	- dir: Path of the source directory, has to exist.
	- zip: Path of the zip file, gets overwritten.
	- incl: If true, the folder itself will be added into the zip file.*/
	bool ZipFolder(std::string dir, std::string zip, bool incl = false);
	/*UnzipFolder() unzips an entire folder. Uses ZLib.
	Returns false, if zip and dir are identical, zip doesn't exist or dir can't be written. True on success.
	- zip: Path to the source zip file, has to exist.
	- file: Path to the folder, gets overwritten.
	- incl: If true, the folder itself will be added into the zip file.*/
	bool UnzipFolder(std::string zip, std::string dir, bool incl = false);
};