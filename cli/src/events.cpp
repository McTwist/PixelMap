#include "events.hpp"

#include "platform.hpp"

#include <vector>

static void interupt();

#if defined(PLATFORM_WINDOWS)
#include <windows.h>

// Windows events
BOOL WINAPI win_event(DWORD type)
{
	switch (type)
	{
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_C_EVENT:
		interupt();
		return true;
	default:
		return false;
	}
}

// Check if event have registered
static bool registeredEvent = false;
// Register an event
// Note: Ignores the input, as it is not used
static void registerEvent(int)
{
	if (registeredEvent)
		return;
	SetConsoleCtrlHandler(win_event, TRUE);
	registeredEvent = true;
}
// Dummy enums
enum Event
{
	EVENT_INTERUPT
};

#elif defined(PLATFORM_UNIX)
#include <signal.h>

// Unix events
static void unix_event(int type)
{
	switch (type)
	{
	case SIGINT: interupt(); break;
	default: break;
	}
}

// Register an event
static void registerEvent(int type)
{
	struct sigaction sigHandler;

	sigHandler.sa_handler = unix_event;
	sigemptyset(&sigHandler.sa_mask);
	sigHandler.sa_flags = 0;

	sigaction(type, &sigHandler, NULL);
}

// The event to register
enum Event
{
	EVENT_INTERUPT = SIGINT
};

#endif // PLATFORM_UNIX

/*
Internal static data
*/
static std::vector<Events::eventFunc> funcInterupt;

namespace Events
{

// Register the interupt
void registerInterupt(eventFunc func)
{
	// Register first
	if (funcInterupt.empty())
	{
		registerEvent(EVENT_INTERUPT);
	}
	funcInterupt.push_back(func);
}

}

/**
 * @brief Interupt event handling
 */
static void interupt()
{
	for (auto func : funcInterupt)
		func();
}
