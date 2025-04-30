#pragma once
#ifndef WEBVIEW_HPP
#define WEBVIEW_HPP

#include <string>

namespace WebView
{
	void createDefaultRoot(const std::string & path);
	std::string getRegionFolder(const std::string & path, int zoom);
}

#endif // WEBVIEW_HPP
