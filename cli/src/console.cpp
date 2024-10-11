#include "console.hpp"

#include "platform.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>

#include <cstring>
#include <cstdio>

// Calculate the moving average whenever a value is updated.
// This is to reduce the movement significantly to avoid jitter.
class MovingAverage
{
	const float EPSILON = 0.1f;
	const float ALPHA = 0.1f;
public:
	float update(float next)
	{
		float high = prev * (1 + EPSILON);
		float low = prev * (1 - EPSILON);

		// Reset or smooth
		prev = (next > high || next < low) ? next : prev * (1 - ALPHA) + next * ALPHA;
		return prev;
	}
private:
	float prev = 0.f;
};

#ifdef PLATFORM_WINDOWS
#include <windows.h>

struct Data
{
	HANDLE console;
	MovingAverage average;
};

#elif defined(PLATFORM_UNIX)
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#define ESCAPE_CODE "\x1B"
#define ESCAPE_POS "["

struct Data
{
	MovingAverage average;
};

#endif // PLATFORM_UNIX

constexpr unsigned int TIME_MINUTE = 60;
constexpr unsigned int TIME_HOUR = 60 * TIME_MINUTE;
static std::string timeToString(uint64_t t);

Console::Console()
{
	std::ios::sync_with_stdio(false);
	data = std::make_shared<Data>();
#ifdef PLATFORM_WINDOWS
	data->console = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

void Console::progress(uint32_t count, uint32_t current, float elapsed, const std::string & status)
{
	current = (std::min)(current, count);
	float percent = (count == 0) ? 0 : float(current) / count;
	float distance = (percent <= 0.000001f) ? 0 : elapsed / percent;
	elapsed = data->average.update(distance) * percent;
	int left = (percent <= 0.000001f) ? 0 : int(distance - elapsed);

	auto time = timeToString(left);

	if (time.size() < 7)
		time = std::string(7 - time.size(), ' ') + time;

	int width, y;
	std::tie(width, std::ignore) = size();
	std::tie(std::ignore, y) = cursor();

	setCursorPosition(0, y-1);

	auto barWidth = width - (2 + time.size() + 2 + status.size()); // edges plus time
	auto widthLeft = decltype(barWidth)(barWidth * percent);
	auto widthRight = barWidth - widthLeft;

	auto barLeft = (widthLeft == 0) ? std::string() : std::string(widthRight == 0 ? widthLeft : widthLeft - 1, '=');
	auto barCursor = (widthLeft == 0 || widthRight == 0) ? "" : ">";
	auto barRight = std::string(widthRight, ' ');

	std::cout << status << "[" << barLeft << barCursor << barRight << "]" << " " << time << std::flush;
}

void Console::newLine()
{
	std::cout << std::endl;
}

std::tuple<int, int> Console::size() const
{
#ifdef PLATFORM_WINDOWS
	CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
	GetConsoleScreenBufferInfo(data->console, &csbi);
	return { csbi.srWindow.Right - csbi.srWindow.Left + 1, csbi.srWindow.Bottom - csbi.srWindow.Top + 1 };
#elif defined(PLATFORM_UNIX)
	struct winsize size = { 0 };
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	return { size.ws_col, size.ws_row };
#else
	return {0, 0};
#endif
}

std::tuple<int, int> Console::cursor() const
{
#ifdef PLATFORM_WINDOWS
	CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
	GetConsoleScreenBufferInfo(data->console, &csbi);
	return { csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y };
#elif defined(PLATFORM_UNIX)

	struct termios save, raw;
	std::memset(&raw, 0, sizeof(raw));
	tcgetattr(STDIN_FILENO, &save);
	cfmakeraw(&raw);
	tcsetattr(STDIN_FILENO, TCSANOW, &raw);
	isatty(STDIN_FILENO);

	std::cout << ESCAPE_CODE << ESCAPE_POS << "6n" << std::flush;
	//std::string in;
	//std::cin >> in;
	int x, y;
	auto c = std::fscanf(stdin, ESCAPE_CODE ESCAPE_POS "%d;%dR", &y, &x);

	tcsetattr(STDIN_FILENO, TCSANOW, &save);

	if (c != 2)
		return { 0, 0 };
	// Note: Fix this
	return { x, y };
#else
	return { 0, 0 };
#endif
}

void Console::setCursorPosition(unsigned int col, unsigned int row)
{
#ifdef PLATFORM_WINDOWS
	COORD pos = {SHORT(col), SHORT(row)};
	SetConsoleCursorPosition(data->console, pos);
#elif defined(PLATFORM_UNIX)
	std::cout << ESCAPE_CODE << ESCAPE_POS << (row+1) << ";" << (col+1) << "H" << std::flush;
#endif
}

void Console::clearLine()
{
#ifdef PLATFORM_WINDOWS
	auto [x, y] = cursor();
	auto [width, _] = size();
	setCursorPosition(0, y);
	std::cout << std::string(width, ' ') << std::flush;
	setCursorPosition(x, y);
#elif defined(PLATFORM_UNIX)
	std::cout << ESCAPE_CODE << ESCAPE_POS << "K" << std::flush;
#endif
}

bool Console::supportANSI() const
{
#ifdef PLATFORM_WINDOWS
	return false;
#elif defined(PLATFORM_UNIX)
	return true;
#else
	return false;
#endif
}

inline std::string timeToString(uint64_t t)
{
	std::stringstream stream;
	uint64_t hour = t / TIME_HOUR;
	uint64_t minute = (t % TIME_HOUR) / TIME_MINUTE;
	uint64_t second = (t % TIME_MINUTE);
	if (hour >= 1)
		stream << hour << ":";
	stream << std::setfill('0') << std::setw(2)
		<< minute << ":"
		<< std::setfill('0') << std::setw(2)
		<< second;
	return stream.str();
}
