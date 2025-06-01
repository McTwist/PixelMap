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

	void progress(uint32_t count, uint32_t current, const std::string & status = "");

private:
	std::shared_ptr<struct Data> data;
};

#endif // CONSOLE_HPP
