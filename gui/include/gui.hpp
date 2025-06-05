#ifndef GUI_HPP
#define GUI_HPP

#include <vector>
#include <array>
#include <string>
#include <future>

class GUI
{
	GUI() = delete;
public:
	// Helpers
	static bool Combo(const char * label, std::size_t * current_item, const std::vector<std::string> & items);
	static std::future<bool> BrowseLoad(const char * label,
		std::string & outPath,
		const std::vector<std::array<std::string, 2>> & items,
		const std::string & defaultPath = std::string());
	static std::future<bool> BrowseSave(const char * label,
		std::string & outPath,
		const std::vector<std::array<std::string, 2>> & items,
		const std::string & defaultPath = std::string(),
		const std::string & defaultName = std::string());
	static std::future<bool> BrowseFolder(const char * label,
		std::string & outPath,
		const std::string & defaultPath = std::string());

	static bool OpenDialog(std::string & outPath,
		const std::vector<std::array<std::string, 2>> & items,
		const std::string & defaultPath = std::string());
	static bool SaveDialog(std::string & outPath,
		const std::vector<std::array<std::string, 2>> & items,
		const std::string & defaultPath = std::string(),
		const std::string & defaultName = std::string());
	static bool PickFolder(std::string & outPath,
		const std::string & defaultPath = std::string());
};

#endif // GUI_HPP
