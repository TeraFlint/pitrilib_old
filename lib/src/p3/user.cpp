#include "user.h"
#include "filemanip.h"
#include <fstream>

namespace Pitri
{
	void UserData::SetLocal(std::string val)
	{
		local = val;
		has_local = !val.empty();
	}
	std::string UserData::GetLocal() const
	{
		return local;
	}
	bool UserData::HasLocal() const
	{
		return has_local;
	}
	void UserData::DeleteLocal()
	{
		local = "";
		has_local = false;
	}

	void UserData::SetGlobal(std::string val)
	{
		global = val;
		has_global = !val.empty();
	}
	std::string UserData::GetGlobal() const
	{
		return global;
	}
	bool UserData::HasGlobal() const
	{
		return has_global;
	}
	void UserData::DeleteGlobal()
	{
		local = "";
		has_global = false;
	}

	std::ostream &operator<<(std::ostream &out, const UserData &data)
	{
		if (data.has_local)
		{
			out << "\"" << data.local << "\"";
			if (data.has_global)
				out << " ";
		}
		if (data.has_global)
			out << "(\"" << data.global << "\")";
		return out;
	}


	std::string User::config_path = "";
	std::string User::config_dev_path = "";
	std::string User::config_app_path = "";
	bool User::default_path = false;
	bool User::init_config = false;

	bool User::Zip()
	{
		if (user_id.empty())
			return false;
		const std::string src = GetPath(false);
		const std::string dst = GetPath(true);

		//Todo:
		//Zip. Needs zlib added to the library.
		//Then delete unpacked files, if successful.
		//...

		zipped = true;
		return true;
	}
	bool User::Unzip()
	{
		if (user_id.empty())
			return false;
		const std::string src = GetPath(true);
		const std::string dst = GetPath(false);

		//Todo:
		//Delete all files in unpacked\user_id.
		//Then unzip. Needs zlib added to the library.
		//...

		zipped = false;
		return true;
	}

	bool User::AdjustImage(Image &img, bool centered, unsigned size)
	{
		if (img.Width() == size && img.Height() == size)
			return false;

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
			success |= ImageEditor::Resize(img, width, height);
		}
		//If it's not correct now, it's smaller.
		if (img.Width() != size || img.Height() != size)
		{
			int x = 0, y = 0;
			if (!centered)
				x = -100;

			success |= ImageEditor::ResizeCanvas(img, size, size, x, y, false);
		}
		return success;
	}

	std::string User::GetCustomPath(std::string dir, bool global, bool app_path) const
	{
		if (dir.empty()) return "";
		if (dir[dir.size() - 1] != OS_SLASH)
			dir += OS_SLASH;

		std::string app = "";
		if (!config_app_path.empty() && app_path)
			app = config_app_path + OS_SLASH;

		const std::string config = GetConfigPath();
		if (global) return config + dir + app;

		if (user_id.empty()) return "";
		return config + "users" + OS_SLASH + "unpacked" + OS_SLASH + user_id + OS_SLASH + dir + app;
	}
	std::string User::GetFirstFileLine(std::string path)
	{
		std::ifstream file(path);
		if (file.good())
		{
			std::string line;
			bool comment = false;
			while (std::getline(file, line))
			{
				line = TrimString(ParseComment(line, &comment));
				if (!line.empty())
				{
					return line;
					file.close();
				}
			}
			file.close();
		}
		return "";
	}

	User::User()
	{
		core_change = false;
		image_change = false;
		settings_change = false;
		zipped = false;
		autosave = true;

		has_icon = has_flag = has_emblem = false;
	}
	User::User(std::string id) : User()
	{
		user_id = id;
	}
	User::~User()
	{
		if (autosave && !zipped && !user_id.empty())
		{
			if (core_change)
				SaveCore(user_id, false);
			if (image_change)
				SaveImages();
			if (settings_change)
				SaveSettings();
			Zip();
		}
	}

	bool User::Load(std::string id, bool from_zip)
	{
		LoadCore(id, from_zip);
		LoadImages();
		LoadSettings();
		return true;
	}
	bool User::Save(std::string id, bool to_zip)
	{
		SaveCore(id, false);
		SaveImages();
		SaveSettings();
		if (to_zip)
			Zip();
		core_change = false;
		return true;
	}

	bool User::LoadCore(std::string id, bool from_zip)
	{
		if (!id.empty())
			user_id = id;
		if (from_zip)
			Unzip();
		core_change = false;
		std::string path = GetPath(false);
		auto contents = ScanSectorFile(path + "core.usr");
		for (auto sector : contents)
		{
			for (auto entry : sector.entries)
			{
				core[entry.first] = entry.second;
			}
		}

		return true;
	}
	bool User::SaveCore(std::string id, bool to_zip)
	{
		if (!id.empty())
			user_id = id;

		std::string path = GetPath(false);
		CreateDirectoryPath(path);
		std::ofstream file(path + "core.usr");
		if (file.good())
		{
			for (auto entry : core)
			{
				file << entry.first << "=" << entry.second << std::endl;
			}
			file.close();
		}

		if (to_zip)
			Zip();
		core_change = false;
		return true;
	}

	bool User::LoadImages()
	{
		if (user_id.empty() || zipped)
			return false;

		image_change = false;
		has_icon = has_flag = has_emblem = 0;

		std::string path = GetPath(false);
		if (icon.Load(path + "icon.png"))
		{
			has_icon = true;
			if (AdjustImage(icon))
				image_change = true;
		}
		if (flag.Load(path + "flag.png"))
		{
			has_flag = true;
			if (AdjustImage(flag, true))
				image_change = true;
		}
		if (emblem.Load(path + "emblem.png"))
		{
			has_emblem = true;
			if (AdjustImage(emblem))
				image_change = true;
		}
		return true;
	}
	bool User::SaveImages()
	{
		if (user_id.empty())
			return false;

		std::string path = GetPath(false);
		CreateDirectoryPath(path);
		if (has_icon)
			icon.Save(path + "icon.png");
		if (has_flag)
			flag.Save(path + "flag.png");
		if (has_emblem)
			emblem.Save(path + "emblem.png");
		image_change = false;
		return true;
	}

	bool User::LoadSettings()
	{
		if (user_id.empty() || zipped)
			return false;
		settings_change = false;

		const std::string ext = ".cfg";
		const std::string local_path = GetSettingsPath(false);
		const std::string global_path = GetSettingsPath(true);

		for (auto file : ScanDirectory(local_path, DT_REG))
		{
			int offset = file.size() - ext.size();
			if (offset > 0 && !file.substr(offset).compare(ext))
			{
				std::string id = file.substr(0, offset);
				for (auto sector : ScanSectorFile(local_path + file))
				{
					for (auto entry : sector.entries)
					{
						if (!entry.first.empty() && !entry.second.empty())
							settings[id][entry.first].SetLocal(entry.second);
					}
				}
			}
		}
		for (auto file : ScanDirectory(global_path, DT_REG))
		{
			int offset = file.size() - ext.size();
			if (offset > 0 && !file.substr(offset).compare(ext))
			{
				std::string id = file.substr(0, offset);
				for (auto sector : ScanSectorFile(global_path + file))
				{
					for (auto entry : sector.entries)
					{
						if (!entry.first.empty() && !entry.second.empty())
							settings[id][entry.first].SetGlobal(entry.second);
					}
				}
			}
		}
		return true;
	}
	bool User::SaveSettings()
	{
		if (user_id.empty())
			return false;

		const std::string local_path = GetSettingsPath(false);
		const std::string global_path = GetSettingsPath(true);
		if (!CreateDirectoryPath(local_path) || !CreateDirectoryPath(global_path))
			return false;

		const std::string ext = ".cfg";
		for (auto filename : settings)
		{
			std::map<std::string, std::string> local_settings, global_settings;
			std::string local_file = local_path + filename.first + ext;
			std::string global_file = global_path + filename.first + ext;

			for (auto entry : filename.second)
			{
				if (entry.second.HasLocal())
				{
					std::string content = entry.second.GetLocal();
					if(!content.empty())
						local_settings[entry.first] = content;
				}
				if (entry.second.HasGlobal())
				{
					std::string content = entry.second.GetGlobal();
					if (!content.empty())
						global_settings[entry.first] = content;
				}
			}

			if (!local_settings.empty())
			{
				std::ofstream file(local_file);
				if (file.good())
				{
					for (auto entry : local_settings)
					{
						file << entry.first << "=" << entry.second << std::endl;
					}
				}
			}
			else DeleteFile(local_file.c_str());

			if (!global_settings.empty())
			{
				std::ofstream file(global_file);
				if (file.good())
				{
					for (auto entry : global_settings)
					{
						file << entry.first << "=" << entry.second << std::endl;
					}
				}
			}
			else DeleteFile(global_file.c_str());
		}

		settings_change = false;
		return true;
	}

	std::string User::GetSettingsPath(bool global) const
	{
		return GetCustomPath("settings", global);
	}
	std::string User::GetSavegamePath(bool global) const
	{
		return GetCustomPath("saves", global);
	}
	std::string User::GetPath(bool zip) const
	{
		if (user_id.empty())
			return "";

		const std::string users = GetConfigPath() + "users" + OS_SLASH;
		if (zip)
			return users + "packed" + OS_SLASH + user_id + ".zip";
		return users + "unpacked" + OS_SLASH + user_id + OS_SLASH;
	}

	bool User::GenerateStructure(bool app_folder)
	{
		std::string app = "";
		if (app_folder && !config_app_path.empty())
			app = config_app_path + OS_SLASH;

		std::vector<std::string> dirs = { std::string("settings") + OS_SLASH + app, std::string("saves") + OS_SLASH + app, std::string("users") + OS_SLASH + "packed" + OS_SLASH };
		if (!user_id.empty())
		{
			for (unsigned i = 0; i < 2; ++i)
			{
				dirs.push_back(std::string("users") + OS_SLASH + "unpacked" + OS_SLASH + user_id + OS_SLASH + dirs[i]);
			}
		}
		else
			dirs.push_back(std::string("users") + OS_SLASH + "unpacked" + OS_SLASH);

		bool success = true;
		std::string cfg_path = GetConfigPath();
		for (auto dir : dirs)
		{
			std::string path = cfg_path + dir;
			success &= CreateDirectoryPath(path);
		}
		return success;
	}
	void User::SetAutoSave(bool save)
	{
		autosave = save;
	}

	std::string User::GetID()
	{
		return user_id;
	}
	void User::SetID(std::string id)
	{
		user_id = id;
	}

	bool User::SetCore(std::string key, std::string value)
	{
		if (key.empty())
			return false;

		auto entry = core.find(key);
		if (entry == core.end())
			core_change = true;
		else if (entry->second.compare(value))
			core_change = true;
		core[key] = value;
		return true;
	}
	bool User::HasCore(std::string key) const
	{
		return !key.empty() && core.find(key) != core.end();
	}
	bool User::RemoveCore(std::string key)
	{
		if (key.empty() || core.find(key) == core.end())
			return false;

		core.erase(key);
		core_change = true;
		return true;
	}
	std::string User::GetCore(std::string key) const
	{
		auto entry = core.find(key);
		return entry->second;
	}
	std::vector<std::string> User::GetCoreKeys() const
	{
		std::vector<std::string> result;
		for (auto entry : core)
		{
			if (!entry.first.empty())
				result.push_back(entry.first);
		}
		return result;
	}

	bool User::SetIcon(const Image &img)
	{
		icon = img;
		AdjustImage(icon, false);
		has_icon = true;
		image_change = true;
		return true;
	}
	bool User::HasIcon() const
	{
		return has_icon;
	}
	bool User::RemoveIcon()
	{
		if (!has_icon)
			return false;
		icon = Image(0, 0);
		image_change |= has_icon;
		has_icon = false;
		return true;
	}
	Image User::GetIcon() const
	{
		return icon;
	}

	bool User::SetFlag(const Image &img)
	{
		flag = img;
		AdjustImage(flag, true);
		has_flag = true;
		image_change = true;
		return true;
	}
	bool User::HasFlag() const
	{
		return has_flag;
	}
	bool User::RemoveFlag()
	{
		if (!has_flag)
			return false;
		flag = Image(0, 0);
		image_change |= has_flag;
		has_flag = false;
		return true;
	}
	Image User::GetFlag() const
	{
		return flag;
	}

	bool User::SetEmblem(const Image &img)
	{
		emblem = img;
		AdjustImage(emblem, false);
		has_emblem = true;
		image_change = true;
		return true;
	}
	bool User::HasEmblem() const
	{
		return has_emblem;
	}
	bool User::RemoveEmblem()
	{
		if (!has_emblem)
			return false;
		emblem = Image(0, 0);
		image_change |= has_emblem;
		has_emblem = false;
		return true;
	}
	Image User::GetEmblem() const
	{
		return emblem;
	}

	bool User::SetSettings(std::string file, std::string key, std::string value, bool global)
	{
		if (file.empty() || key.empty())
			return false;

		if (!settings_change)
		{
			auto filename = settings.find(file);
			if (filename != settings.end())
			{
				auto &entry = filename->second.find(key);
				if (entry != filename->second.end())
				{
					auto &data = entry->second;
					if (global && data.GetGlobal().compare(value) || !global && data.GetLocal().compare(value))
						settings_change = true;
				}
				else settings_change = true;
			}
			else settings_change = true;
		}

		auto &data = settings[file][key];
		if (global)
			data.SetGlobal(value);
		else
			data.SetLocal(value);
		return true;
	}
	bool User::HasSettings(std::string file, std::string key, bool global)
	{
		if (!HasSettings(file, key))
			return false;

		auto entry = settings.at(file).at(key);
		if (global)
			return entry.HasGlobal();
		else
			return entry.HasLocal();
	}
	bool User::HasSettings(std::string file, std::string key) const
	{
		if (file.empty() || key.empty())
			return false;

		auto &filename = settings.find(file);
		if (filename == settings.end())
			return false;
		auto &entry = filename->second.find(key);
		return entry != filename->second.end();
	}
	bool User::RemoveSettings(std::string file)
	{
		if (file.empty() || settings.find(file) == settings.end())
			return false;
		settings.erase(file);
		settings_change = true;
		return true;
	}
	bool User::RemoveSettings(std::string file, std::string key, bool global)
	{
		if (file.empty() || settings.find(file) == settings.end())
			return false;

		auto &entries = settings[file];
		if (key.empty() || entries.find(key) == entries.end())
			return false;

		auto &entry = entries[key];
		if (global)
		{
			if (!entry.HasGlobal()) return false;
			if (entry.HasLocal()) entry.DeleteGlobal();
			else entries.erase(key);
		}
		else
		{
			if (!entry.HasLocal()) return false;
			if (entry.HasGlobal()) entry.DeleteLocal();
			else entries.erase(key);
		}
		settings_change = true;
		return true;
	}
	std::string User::GetSettings(std::string file, std::string key, bool global) const
	{
		if (file.empty() || key.empty())
			return "";

		auto filename = settings.find(file);
		if (filename != settings.end())
		{
			auto entry = filename->second.find(key);
			if (entry != filename->second.end())
			{
				return global ? entry->second.GetGlobal() : entry->second.GetLocal();
			}
		}
		return "";
	}
	std::vector<std::string> User::GetSettingsFiles() const
	{
		std::vector<std::string> result;
		for (auto entry : settings)
		{
			if (!entry.first.empty())
				result.push_back(entry.first);
		}
		return result;
	}
	std::vector<std::string> User::GetSettingsKeys(std::string file) const
	{
		std::vector<std::string> result;
		if (!file.empty())
		{
			auto filename = settings.find(file);
			if (filename != settings.end())
			{
				for (auto entry : filename->second)
				{
					if (!entry.first.empty())
						result.push_back(entry.first);
				}
			}
		}
		return result;
	}

	std::ostream &operator<<(std::ostream &out, const User &user)
	{
		const std::string space = " - ";
		bool change = user.core_change || user.image_change || user.settings_change;
		out << "=== User: " << user.user_id << (change ? " *" : "") << " ===" << std::endl;
		out << (user.image_change ? "* " : "") << "Images: ";
		if (user.has_icon || user.has_flag || user.has_emblem)
		{
			bool first = true;
			if (user.has_icon)
			{
				out << "icon";
				first = false;
			}
			if (user.has_flag)
			{
				if (first) first = false;
				else out << ", ";
				out << "flag";
			}
			if (user.has_emblem)
			{
				if (first) first = false;
				else out << ", ";
				out << "emblem";
			}
			out << ".";
		}
		else out << "-";
		out << std::endl;

		out << (user.core_change ? "* " : "") << "Core attributes: ";
		if (!user.core.empty())
		{
			for (auto entry : user.core)
			{
				out << std::endl << space << entry.first << " = \"" << entry.second << "\"";
			}
		}
		else out << "-";
		out << std::endl;

		if (!user.settings.empty())
		{
			for (auto filename : user.settings)
			{
				out << (user.settings_change ? "* " : "") << "Settings: " << filename.first << " ";
				
				if (!filename.second.empty())
				{
					for (auto entry : filename.second)
					{
						out << std::endl << space << entry.first << " = " << entry.second;
					}
				}
				else out << "-";
				out << std::endl;
			}
		}
		return out;
	}

	std::vector<std::string> User::GetUserList(bool zip)
	{
		std::string path = GetConfigPath();
		std::vector<std::string> result;
		if (zip)
		{
			std::string path = GetConfigPath() + "users" + OS_SLASH + "packed" + OS_SLASH;
			auto files = ScanDirectory(path, DT_REG);
			for (auto file : files)
			{
				int offset = file.size() - 4;
				if (offset > 0 && !file.substr(offset).compare(".zip"))
					result.push_back(file.substr(0, offset));
			}
		}
		else
		{
			std::string path = GetConfigPath() +"users" + OS_SLASH + "unpacked" + OS_SLASH;
			auto dirs = ScanDirectory(path, DT_DIR);
			for (auto dir : dirs)
			{
				std::ifstream file(path + dir + OS_SLASH + "core.usr");
				if (file.good())
					result.push_back(dir);
			}
		}
		return result;
	}
	std::string User::GetDefaultUser()
	{
		return GetFirstFileLine(GetConfigPath() + "users" + OS_SLASH + "default.usr");
	}
	bool User::SetDefaultUser(std::string user)
	{
		std::string path = GetConfigPath() + "users" + OS_SLASH;
		CreateDirectoryPath(path);
		std::ofstream file(path + "default.usr");
		if (!file.good())
			return false;
		file << user;
		file.close();
		return true;
	}

	std::string User::GetConfigAppPath()
	{
		return config_app_path;
	}
	void User::SetConfigAppPath(std::string path)
	{
		config_app_path = path;
		InitConfigPath();
	}

	std::string User::GetConfigDevPath()
	{
		return config_dev_path;
	}
	void User::SetConfigDevPath(std::string path)
	{
		config_dev_path = path;
		InitConfigPath();
	}

	std::string User::GetAppDataPath()
	{
		TCHAR path[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
		{
			std::string result = path;
			if(!result.empty())
				return result + OS_SLASH;
		}
		return "";
	}
	std::string User::GetConfigPath()
	{
		if (!init_config)
			InitConfigPath();

		std::string result = config_path;
		if (!result.empty() && result[result.size() - 1] != OS_SLASH)
			result += OS_SLASH;
		if (default_path && !config_dev_path.empty())
		{
			result += config_dev_path;
			if (!result.empty() && result[result.size() - 1] != OS_SLASH)
				result += OS_SLASH;
		}
		return result;
	}
	unsigned User::FollowConfigPath(std::string &path)
	{
		unsigned hops = 0;
		const std::string redirect = "redirect.txt";
		std::vector<std::string> history;

		for (bool end = false, duplicate = false; !end && !duplicate; )
		{
			if (!path.empty() && path[path.size() - 1] != OS_SLASH)
				path += OS_SLASH;
			history.push_back(path);

			std::string line = GetFirstFileLine(path + redirect);
			if (!line.empty())
			{
				if (line[line.size() - 1] != OS_SLASH)
					line += OS_SLASH;
				for (auto dir : history)
				{
					if (!dir.compare(line))
					{
						duplicate = true;
						break;
					}
				}
				if (!duplicate)
				{
					path = line;
					++hops;
				}
			}
			else
				end = true;
		}
		return hops;
	}
	void User::InitConfigPath()
	{
		default_path = false;
		config_path = "";
		if (!FollowConfigPath(config_path) && !config_dev_path.empty())
		{
			std::string appdata = GetAppDataPath();
			if (!appdata.empty())
			{
				config_path = appdata + config_dev_path;
				if (!FollowConfigPath(config_path))
				{
					config_path = appdata;
					default_path = true;
				}
			}
		}
		init_config = true;
	}
}