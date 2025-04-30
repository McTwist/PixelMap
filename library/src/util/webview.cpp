#include "util/webview.hpp"

#include "util/compression.hpp"
#include "util/endianess.hpp"
#include "platform.hpp"
#include "vectorview.hpp"

#include "webview_fgz.hpp"

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <fstream>

struct Node
{
	std::string_view name;
	VectorView<const uint8_t> data;
};

static auto loadFiles(const std::vector<uint8_t> & data)
{
	std::vector<Node> files;

	auto p = data.data();

	for (std::size_t i = 0; i < data.size();)
	{
		auto name_len = endianess::fromBig<uint32_t>(p+i);
		i += sizeof(name_len);
		const std::string_view name{reinterpret_cast<const char *>(p+i), name_len};
		i += name_len;
		auto data_len = endianess::fromBig<uint32_t>(p+i);
		i += sizeof(data_len);
		VectorView<const uint8_t> _data{p+i, data_len};
		i += data_len;
		files.emplace_back(Node{name, _data});
	}

	return files;
}

void WebView::createDefaultRoot(const std::string & path)
{
	auto data = Compression::loadGZip(VectorView<const uint8_t>(webview_fgz_data, webview_fgz_size));
	auto files = loadFiles(data);
	platform::path::mkdir(path);
	for (auto & file : files)
	{
		auto new_path = platform::path::join(path, std::string{file.name});
		std::ofstream o;
		o.open(new_path, std::ofstream::binary);
		o.write(reinterpret_cast<const char *>(file.data.data()), file.data.size());
		o.close();
	}
}

std::string WebView::getRegionFolder(const std::string & path, int zoom)
{
	return platform::path::join(path, "data", std::to_string(zoom));
}

