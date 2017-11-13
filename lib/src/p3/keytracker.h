#pragma once
#include <map>

namespace Pitri
{
	class KeyTracker
	{
	private:
		std::map<std::string, bool> pressed;
		std::map<std::string, std::string> redirect;
	};
}