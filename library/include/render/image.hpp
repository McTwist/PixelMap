#pragma once
#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "render/utility.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <functional>
#include <memory>

class Image
{
	typedef std::function<void(uint32_t, std::vector<utility::RGBA>&)> Writer;
	friend class ImageWriter;
public:

	explicit Image(uint32_t width, uint32_t height);
	// Fall-through constructor for any value that can convert to uint32_t
	template<typename A, typename B>
	Image(A a, B b) : Image(uint32_t(a), uint32_t(b)) {}

	bool save(const std::string & file, Writer func, const std::string & comment = "");

private:

	std::shared_ptr<class ImageWriter> writer;

};

#endif // IMAGE_HPP
