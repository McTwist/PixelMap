#pragma once
#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <tuple>
#include <memory>
#include <stdint.h>
#include <string>

class Console
{
public:
	Console();

	void progress(uint32_t count, uint32_t current, float elapsed, const std::string & status = "");
	void newLine();

private:
	std::tuple<int, int> size() const;
	std::tuple<int, int> cursor() const;
	void setCursorPosition(unsigned int col, unsigned int row);
	void clearLine();
	bool supportANSI() const;

	std::shared_ptr<struct Data> data;
};

#endif // CONSOLE_HPP
