#include "zip.h"
#include "filemanip.h"

namespace Pitri
{
	bool ZipFile(std::string file, std::string zip)
	{
		if (!zip.compare(file))
			return false;

		//Some actual code. todo: add zlib
		//...
		return false;
	}
	bool UnzipFile(std::string zip, std::string file)
	{
		if (!zip.compare(file))
			return false;

		//Some actual code. todo: add zlib
		//...
		return false;
	}

	bool ZipFolder(std::string dir, std::string zip, bool inc)
	{
		if (!zip.compare(dir))
			return false;

		//Some actual code. todo: add zlib
		//...
		return false;
	}
	bool UnzipFolder(std::string zip, std::string dir, bool inc)
	{
		if (!zip.compare(dir))
			return false;

		CreateDirectoryPath(dir);
		//Some actual code. todo: add zlib
		//...
		return false;
	}
};