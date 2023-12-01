#define _CRT_SECURE_NO_WARNINGS // fopen
#include "render/image.hpp"

#include "platform.hpp"

#include <png.h>

#include <vector>
#include <fstream>

class AtEnd
{
public:
	AtEnd() = delete;
	AtEnd(const std::function<void()> & func) : end(func) {}
	~AtEnd(){ end(); }
private:
	std::function<void()> end;
};

class ImageWriter
{
public:
	ImageWriter(uint32_t _width, uint32_t _height)
		: width(_width), height(_height)
	{
	}

	void write(const std::string & path, Image::Writer func)
	{
		// This works
		auto png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if (!png)
			throw std::runtime_error("Unable to create png");
		AtEnd a1([&png]() { png_destroy_write_struct(&png, nullptr); });
		auto info = png_create_info_struct(png);
		AtEnd a2([png, &info]() { png_destroy_info_struct(png, &info); });

		if (png_get_user_width_max(png) < width || png_get_user_height_max(png) < height)
			throw std::runtime_error("Image are too large");

		platform::fd::enter();
		auto fd = fopen(path.c_str(), "wb");
		if (!fd)
			throw std::runtime_error("Unable to write image");
		AtEnd a3([fd]()
		{
			fclose(fd);
			platform::fd::leave();
		});
		png_init_io(png, fd);

		png_set_IHDR(png, info, width, height,
					 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
					 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		png_write_info(png, info);

		using PngByte = png_byte *;

		for (decltype(height) i = 0; i < height; ++i)
		{
			png_write_row(png, PngByte(func(i).data()));
		}

		png_write_end(png, nullptr);
	}

private:
	uint32_t width, height;
};

Image::Image(uint32_t width, uint32_t height)
{
	writer = std::make_shared<ImageWriter>(width, height);
}

bool Image::save(const std::string &file, Writer func)
{
	try
	{
		writer->write(file, func);
	}
	catch (const std::exception &)
	{
		return false;
	}
	return true;
}
