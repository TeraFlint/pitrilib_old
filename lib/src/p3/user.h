#pragma once
#include "imagemanip.h"
#include <ShlObj.h>
#include <iostream>

#define P3_USER_IMGSIZE 100

namespace Pitri
{
	class UserData
	{
		private:
			bool has_local, has_global;
			std::string local, global;

		public:
			void SetLocal(std::string val);
			std::string GetLocal() const;
			bool HasLocal() const;
			void DeleteLocal();

			void SetGlobal(std::string val);
			std::string GetGlobal() const;
			bool HasGlobal() const;
			void DeleteGlobal();

			friend std::ostream &operator<<(std::ostream &out, const UserData &data);
	};

	class User
	{
		private:
			bool Zip();
			bool Unzip();

			bool AdjustImage(Image &img, bool centered = true, unsigned size = P3_USER_IMGSIZE);
			std::string GetCustomPath(std::string dir, bool global = false, bool app_path = true) const;
			static std::string GetFirstFileLine(std::string path);

			bool zipped, autosave;
			bool core_change, image_change, settings_change;
			std::string user_id;
			Image icon, flag, emblem;
			bool has_icon, has_flag, has_emblem;
			std::map<std::string, std::string> core;
			std::map<std::string, std::map<std::string, UserData>> settings;

			static std::string config_path;
			static std::string config_dev_path;
			static std::string config_app_path;
			static bool default_path;
			static bool init_config;

		public:
			User();
			User(std::string id);
			~User();

			/*Load() executes LoadCore() and LoadSettings().
			- id: User ID. If not empty, overwrites the previous ID.
			- from_zip: If true, the user will be unzipped first.*/
			bool Load(std::string id = "", bool from_zip = true);
			/*Save() executes SaveCore() and SaveSettings().
			- id: User ID. If not empty, overwrites the previous ID.
			- to_zip: If true, user will be zipped and "unpacked" dir will be cleared.*/
			bool Save(std::string id = "", bool to_zip = true);

			/*LoadCore() loads a user from the "unpacked" directory.
			- id: User ID. If not empty, overwrites the previous ID.
			- from_zip: If true, the user will be unzipped first.*/
			bool LoadCore(std::string id, bool from_zip = true);
			/*SaveCore() saves a user to the "unpacked" directory.
			- id: User ID. If not empty, overwrites the previous ID.
			- to_zip: If true, user will be zipped and "unpacked" dir will be cleared.*/
			bool SaveCore(std::string id, bool to_zip = true);

			/*LoadImages() loads images of the user.*/
			bool LoadImages();
			/*SaveImages() saves images of th user.*/
			bool SaveImages();

			/*LoadSettings() loads the public and private settings of a user.*/
			bool LoadSettings();
			/*SaveSettings() saves the private settings to the user and the public settings to the public directory.*/
			bool SaveSettings();

			/*GetSettingsPath() returns the path of the settings folder.
			- global: determines, if public or private settings.*/
			std::string GetSettingsPath(bool global = false) const;
			/*GetSettingsPath() returns the path of the savegame folder.
			- global: determines, if public or private savegame.*/
			std::string GetSavegamePath(bool global = false) const;
			/*GetPath() returns the path of the zip or the working directory.
			- zip: if true, returns the zip file path (including filename).*/
			std::string GetPath(bool zip = false) const;

			/*GenerateStructure() creates the whole directory structure, including global directories.
			Only creates standard directories, if user_id is not set.
			- app_folder: creates a directory for the program in the settings/savegames folder.*/
			bool GenerateStructure(bool app_folder = false);
			/*SetAutoSave() changes the behaviour of the user on destruction.
			- save: If true, and unsaved changes are made, user saves.*/
			void SetAutoSave(bool save);

			std::string GetID();
			void SetID(std::string id);

			bool SetCore(std::string key, std::string value);
			bool HasCore(std::string key) const;
			bool RemoveCore(std::string key);
			std::string GetCore(std::string key) const;
			std::vector<std::string> GetCoreKeys() const;

			bool SetIcon(const Image &img);
			bool HasIcon() const;
			bool RemoveIcon();
			Image GetIcon() const;

			bool SetFlag(const Image &img);
			bool HasFlag() const;
			bool RemoveFlag();
			Image GetFlag() const;

			bool SetEmblem(const Image &img);
			bool HasEmblem() const;
			bool RemoveEmblem();
			Image GetEmblem() const;

			bool SetSettings(std::string file, std::string key, std::string value, bool global = false);
			bool HasSettings(std::string file, std::string key, bool global);
			bool HasSettings(std::string file, std::string key) const;
			bool RemoveSettings(std::string file);
			bool RemoveSettings(std::string file, std::string key, bool global = false);
			std::string GetSettings(std::string file, std::string key, bool global = false) const;
			std::vector<std::string> GetSettingsFiles() const;
			std::vector<std::string> GetSettingsKeys(std::string file) const;

			friend std::ostream &operator<<(std::ostream &out, const User &user);

			static std::vector<std::string> GetUserList(bool zip);
			static std::string GetDefaultUser();
			static bool SetDefaultUser(std::string user);
			static std::string GetConfigAppPath();
			static void SetConfigAppPath(std::string path);
			static std::string GetConfigDevPath();
			static void SetConfigDevPath(std::string path);
			static std::string GetAppDataPath();
			static std::string GetConfigPath();
			static unsigned FollowConfigPath(std::string &path);
			static void InitConfigPath();
	};
}