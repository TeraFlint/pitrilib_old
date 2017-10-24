#pragma once

#include "filemanip.h"
#include "imagemanip.h"

#include <Windows.h>
#include <map>

#define USER_PROFILE_PICTURE_SIZE 100

namespace Pitri
{
	class UserProfile
	{
	private:
		bool AdjustImage(Image &img, const unsigned size, const bool centered);
		bool AdjustAvatar(std::string path, unsigned size);
		bool AdjustFlag(std::string path, unsigned size);
		bool AdjustEmblem(std::string path, unsigned size);
		std::string GetUserPath();

		std::string userid;
		//strmap attributes;
		std::map<std::string, FileEntry> attributes;
		Image avatar, flag, emblem;

		//Has the userprofile already been read out?
		bool initialized;
		//Are the pictures valid?
		bool valid_avatar, valid_flag, valid_emblem;

	public:
		UserProfile(std::string path);

		bool Load();
		bool Save();
		std::vector<std::string> GetKeyList();
		FileEntry GetKeyValue(std::string key);

		bool HasAvatar() const;
		Image GetAvatar() const;

		bool HasFlag() const;
		Image GetFlag() const;

		bool HasEmblem() const;
		Image GetEmblem() const;

		std::string GetID();
		void ChangeID(std::string id);

		void SetAttribute(std::string key, std::string value);
		void SetAttribute(std::string key, std::vector<std::string> values);
		void SetAttribute(std::string key, FileEntry entry);
	};

	std::string GetAppDataPath();
	std::string GetConfigPath(std::string file = "");

	std::string GetDefaultUser();
	std::vector<std::string> GetUserList();
}