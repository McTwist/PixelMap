#ifndef GUI_HPP
#define GUI_HPP

#include <vector>
#include <array>
#include <string>
#include <functional>
#include <memory>
#include <future>

class GUI
{
	struct Framed
	{
		std::function<void()> begin;
		std::function<void()> end;
	};
public:
	// GUI creation
	void create(const std::string & title, int w, int h);
	void destroy();
	void begin();
	void end();
	bool alive() const { return run; }
	void close() { run = false; }

	// Helpers
	static bool InputText(const char* label, std::string & buf);
	static bool Combo(const char * label, int * current_item, const std::vector<std::string> & items);
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
private:
	std::vector<Framed> frames;
	bool run = true;
	std::shared_ptr<struct Data> data;
};

#endif // GUI_HPP
