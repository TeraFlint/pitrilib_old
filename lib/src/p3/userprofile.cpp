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

	bool UserProfile::AdjustImage(Image &img, const unsigned size, const bool centered)
	{
		bool success = false;
		unsigned width = img.Width(), height = img.Height();
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
			success |=  ImageEditor::Resize(img, width, height);
		}

		//If it's not correct now, it's smaller.
		if (centered && (img.Width() != size || img.Height() != size))
		{
			int x = 0, y = 0;
			if (!centered)
				x = y = -100;
			success |= ImageEditor::ResizeCanvas(img, size, size, x, y, true);
		}
		return success;

		/*
		if (img.Width() != size || img.Height() != size)
		{
			Image square_img(size, size);
			unsigned xoff = (square_img.Width() - img.Width()) / 2, yoff = (square_img.Height() - img.Height()) / 2;
			for (unsigned y = 0; y < img.Height(); ++y)
				for (unsigned x = 0; x < img.Width(); ++x)
					square_img.Pixel(x + xoff, y + yoff) = img.Pixel(x, y);
			img = square_img;
			success = true;
		}
		return success;
		*/
	}

	bool UserProfile::AdjustAvatar(std::string path, unsigned size)
	{
		if (AdjustImage(avatar, size, true))
			if (avatar.Save(path + "\\icon.png"))
				return true;
		return false;
	}

	bool UserProfile::AdjustFlag(std::string path, unsigned size)
	{
		if (AdjustImage(flag, size, false))
			if (flag.Save(path + "\\flag.png"))
				return true;
		return false;
	}
	bool UserProfile::AdjustEmblem(std::string path, unsigned size)
	{
		if (AdjustImage(emblem, size, true))
			if(emblem.Save(path + "\\emblem.png"))
				return true;
		return false;
	}

	std::string UserProfile::GetUserPath()
	{
		if (!userid.size()) return "";
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
				AdjustAvatar(path, size);
		}
		if (flag.Load(path + "\\flag.png"))
		{
			valid_flag = true;
			unsigned size = USER_PROFILE_PICTURE_SIZE;
			if (flag.Width() != size || flag.Height() != size)
				AdjustFlag(path, size);
		}
		if (emblem.Load(path + "\\emblem.png"))
		{
			valid_emblem = true;
			unsigned size = USER_PROFILE_PICTURE_SIZE;
			if (emblem.Width() != size || emblem.Height() != size)
				AdjustEmblem(path, size);
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


	bool UserProfile::HasAvatar() const { return valid_avatar; }
	Image UserProfile::GetAvatar() const { return avatar; }

	bool UserProfile::HasFlag() const { return valid_flag; }
	Image UserProfile::GetFlag() const { return flag; }

	bool UserProfile::HasEmblem() const { return valid_emblem; }
	Image UserProfile::GetEmblem() const { return emblem; }


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

#if defined(P3_CONFIG_DIR)
		std::string folder = P3_CONFIG_DIR;
#else
		std::string folder = "pitrisoftware";
#endif
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

	std::string GetDefaultUser()
	{
		return GetSimpleFileValue(GetConfigPath("users\\defaultuser"), "default");
	}
	
	std::vector<std::string> GetUserList()
	{
		std::vector<std::string> result;
		std::string path = GetConfigPath("users");
		auto entries = ScanDirectory(path, DT_DIR);
		for (auto entry : entries)
		{
			std::ifstream file(path + "\\" + entry + "\\core");
			if (file.good())
				result.push_back(entry);
		}
		return result;
	}
}