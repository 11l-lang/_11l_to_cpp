namespace term
{
#ifdef _WIN32
enum class Color
{
	GRAY    = 0, // Why not `GREY`? ‘Grey’ associates with Sasha Grey. :)(:
	BLUE    = 1,
	GREEN   = 2,
	CYAN    = 3, // [https://github.com/tartley/colorama/blob/master/colorama/winterm.py]
	RED     = 4,
	MAGENTA = 5,
	YELLOW  = 6,
	RESET   = -1
};

static class ConsoleInitializer
{
	HANDLE hConsoleOutput;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

public:
	ConsoleInitializer() {
		GetConsoleScreenBufferInfo(hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	}

	void set_fore_color(WORD c) {
		SetConsoleTextAttribute(hConsoleOutput, c | FOREGROUND_INTENSITY | (csbi.wAttributes & 0xF0));
	}

	void reset_attrs() {
		SetConsoleTextAttribute(hConsoleOutput, csbi.wAttributes);
	}
} con_init;

void color(Color c)
{
	if (c == Color::RESET)
		con_init.reset_attrs();
	else
		con_init.set_fore_color((WORD)c);
}
#else
enum class Color
{
	RED     = 31, // [https://github.com/tartley/colorama/blob/master/colorama/ansi.py]
	GREEN   = 32,
	YELLOW  = 33,
	BLUE    = 34,
	MAGENTA = 35,
	CYAN    = 36,
	GRAY    = 2, // DIM
	RESET   = 0
};

void color(Color c)
{
	if (c != Color::GRAY && c != Color::RESET)
		std::wcout << L"\033[1m"; // BRIGHT
	std::wcout << L"\033[" << int(c) << L'm'; // ]]
}
#endif
}
