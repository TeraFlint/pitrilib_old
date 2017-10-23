#include "userprofile.h"
//#include <lmcons.h> //needed to get the username.
#include <ShlObj.h> //needed to get the appdata file.
#include <fstream>

namespace Pitri
{
	UserProfile::UserProfile(std::string path) : userid(path)
	{
		initialized = false;
		valid_avatar = false;
	}

	void UserProfile::AdjustPicture(std::string path, unsigned size)
	{
		unsigned width = avatar.Width(), height = avatar.Height();
		//Too big? Scale it down.
		if (max(width, height) > size)
		{
			unsigned *bigger = &height, *smaller = &width;
			if (width > height)
			{
				bigger = &width;
				smaller = &height;
			}
			*smaller = size * (*smaller) / (*bigger);
			*bigger = size;

			ImageEditor::SampleDown(avatar, width, height);
		}
		//If it's not correct now, it's smaller.
		if (avatar.Width() != size || avatar.Height() != size)
		{
			Image square(size, size);
			unsigned xoff = (square.Width() - avatar.Width()) / 2, yoff = (square.Height() - avatar.Height()) / 2;
			for (unsigned y = 0; y < avatar.Height(); ++y)
				for (unsigned x = 0; x < avatar.Width(); ++x)
					square.Pixel(x + xoff, y + yoff) = avatar.Pixel(x, y);
			avatar = square;
		}
		avatar.Save(path + "\\icon.png");
	}

	std::string UserProfile::GetUserPath()
	{
		if (!userid.size())
			return "";

		return GetConfigPath("users\\" + userid);
	}

	bool UserProfile::Load()
	{
		if (!userid.size())
			return false;

		std::string path = GetUserPath();
		attributes = ScanSimpleFile(path + "\\core");

		if (avatar.Load(path + "\\icon.png"))
		{
			valid_avatar = true;
			unsigned size = USER_PROFILE_PICTURE_SIZE;
			if (avatar.Width() != size || avatar.Height() != size)
				AdjustPicture(path, size);
		}

		initialized = true;
		return true;
	}

	bool UserProfile::Save()
	{
		if (!initialized || !userid.size())
			return false;

		std::string path = GetUserPath();
		if (!CreateDirectoryPath(path))
			return false;

		std::ofstream file(path + "\\core");
		if (!file.is_open())
			return false;

		file << "[Player]" << std::endl;
		for (auto it : attributes)
		{
			file << it.first;
			if (!it.second.contents.empty())
			{
				file << "=";
				for (unsigned i = 0; i < it.second.contents.size(); ++i)
				{
					if (i) file << ",";
					file << it.second.contents[i];
				}
			}
			file << std::endl;
		}
		file.close();

		if (valid_avatar)
			avatar.Save(path + "\\icon.png");

		return true;
	}

	std::vector<std::string> UserProfile::GetKeyList()
	{
		std::vector<std::string> result;
		if (initialized)
			for (auto it : attributes)
				result.push_back(it.first);
		return result;
	}

	FileEntry UserProfile::GetKeyValue(std::string key)
	{
		if (!initialized)
			return{};
		return attributes[key];
	}


	bool UserProfile::HasPicture() { return valid_avatar; }
	Image UserProfile::GetPicture() { return avatar; }

	std::string UserProfile::GetID() { return userid; }
	void UserProfile::ChangeID(std::string id) { userid = id; }

	void UserProfile::SetAttribute(std::string key, std::string value)
	{
		return SetAttribute(key, Explode(value, ','));
	}
	void UserProfile::SetAttribute(std::string key, std::vector<std::string> values)
	{
		FileEntry entry = {};
		entry.valid = true;
		entry.key = key;
		entry.contents = values;
		return SetAttribute(key, entry);
	}
	void UserProfile::SetAttribute(std::string key, FileEntry entry)
	{
		entry.key = key;
		attributes[key] = entry;
	}

	std::string GetAppDataPath()
	{
		TCHAR path[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
			return path;
		return "";
	}

	std::string CONFIG_PATH = "";
	std::string GetConfigPath(std::string file)
	{
		if (!file.empty())
			file = "\\" + file;

		if (!CONFIG_PATH.empty())
			return CONFIG_PATH + file;

		std::string folder = "pitrisoftware";
		std::string result = GetSimpleFileValue("configpath", "path");
		if (!result.empty())
		{
			CONFIG_PATH = result;
			return result + file;
		}

		result = GetAppDataPath();
		if (!result.empty())
		{
			result += "\\" + folder;
			CONFIG_PATH = result;
			return result + file;
		}
		return "";
	}
}