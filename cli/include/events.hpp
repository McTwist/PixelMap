#pragma once
#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <functional>

namespace Events
{
	// A callback whenever an event occur
	typedef std::function<void()> eventFunc;

	/**
	 * @brief Registers an interupt function
	 * @param eventFunc The function to register
	 */
	void registerInterupt(eventFunc);
};

#endif // EVENTS_HPP
