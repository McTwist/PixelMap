#include "gui.hpp"

#include "imgui/imgui_custom.h"

#include <imgui.h>
#include <nfd.hpp>

#include <algorithm>
#include <future>


bool GUI::Combo(const char * label, std::size_t * current_item, const std::vector<std::string> & items)
{
	const char * preview_value = nullptr;
	if (!items.empty())
		preview_value = items[*current_item].c_str();
	else
		ImGui::BeginDisabled();
	if (ImGui::BeginCombo(label, preview_value))
	{
		for (std::size_t i = 0; i < items.size(); ++i)
		{
			if (ImGui::Selectable(items[i].c_str(), i == *current_item))
				*current_item = i;
		}
		ImGui::EndCombo();
		return true;
	}
	if (items.empty())
		ImGui::EndDisabled();
	return false;
}

std::future<bool> GUI::BrowseLoad(const char * label,
	std::string & outPath,
	const std::vector<std::array<std::string, 2>> & items,
	const std::string & defaultPath)
{
	std::future<bool> ret;
	ImGui::PushID(label);
	ImGui::BeginGroup();
	ImGui::InputText(label, outPath); ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		ret = std::async(std::launch::deferred, [&outPath, items = items, defaultPath = defaultPath]() {
			return OpenDialog(outPath, items, defaultPath);
		});
	}
	ImGui::EndGroup();
	ImGui::PopID();
	return ret;
}

std::future<bool> GUI::BrowseSave(const char * label,
	std::string & outPath,
	const std::vector<std::array<std::string, 2>> & items,
	const std::string & defaultPath,
	const std::string & defaultName)
{
	std::future<bool> ret;
	ImGui::PushID(label);
	ImGui::BeginGroup();
	ImGui::InputText(label, outPath); ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		ret = std::async(std::launch::deferred, [&outPath, items = items, defaultPath = defaultPath, defaultName = defaultName]() {
			return SaveDialog(outPath, items, defaultPath, defaultName);
		});
	}
	ImGui::EndGroup();
	ImGui::PopID();
	return ret;
}

std::future<bool> GUI::BrowseFolder(const char * label,
	std::string & outPath,
	const std::string & defaultPath)
{
	std::future<bool> ret;
	ImGui::PushID(label);
	ImGui::BeginGroup();
	ImGui::InputText(label, outPath); ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		ret = std::async(std::launch::deferred, [&outPath, defaultPath = defaultPath]() {
			return PickFolder(outPath, defaultPath);
		});
	}
	ImGui::EndGroup();
	ImGui::PopID();
	return ret;
}

bool GUI::OpenDialog(std::string & outPath,
	const std::vector<std::array<std::string, 2>> & items,
	const std::string & defaultPath)
{
	NFD::UniquePath _outPath;
	std::vector<nfdfilteritem_t> filterItems;
	filterItems.reserve(items.size());
	std::transform(items.begin(), items.end(), std::back_inserter(filterItems),
		[](const auto & a) { return nfdfilteritem_t{a[0].c_str(), a[1].c_str()}; });
	auto result = NFD::OpenDialog(_outPath,
		filterItems.data(), filterItems.size(),
		defaultPath.empty() ? nullptr : defaultPath.c_str());
	if (result == NFD_OKAY)
	{
		outPath = _outPath.get();
		return true;
	}
	else if (result == NFD_CANCEL)
	{
		return false;
	}
	return false;
}

bool GUI::SaveDialog(std::string & outPath,
	const std::vector<std::array<std::string, 2>> & items,
	const std::string & defaultPath,
	const std::string & defaultName)
{
	NFD::UniquePath _outPath;
	std::vector<nfdfilteritem_t> filterItems;
	filterItems.reserve(items.size());
	std::transform(items.begin(), items.end(), std::back_inserter(filterItems),
		[](const auto & a) { return nfdfilteritem_t{a[0].c_str(), a[1].c_str()}; });
	auto result = NFD::SaveDialog(_outPath,
		filterItems.data(), filterItems.size(),
		defaultPath.empty() ? nullptr : defaultPath.c_str(),
		defaultName.empty() ? nullptr : defaultName.c_str());
	if (result == NFD_OKAY)
	{
		outPath = _outPath.get();
		return true;
	}
	else if (result == NFD_CANCEL)
	{
		return false;
	}
	return false;
}

bool GUI::PickFolder(std::string & outPath,
	const std::string & defaultPath)
{
	NFD::UniquePath _outPath;
	auto result = NFD::PickFolder(_outPath,
		defaultPath.empty() ? nullptr : defaultPath.c_str());
	if (result == NFD_OKAY)
	{
		outPath = _outPath.get();
		return true;
	}
	else if (result == NFD_CANCEL)
	{
		return false;
	}
	return false;
}
