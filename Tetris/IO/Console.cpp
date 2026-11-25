#include "_Console.h"
#include <algorithm>

Console::Console(std::ostream& out, int width, int height)
	: out_(out)
	, width_(width)
	, height_(height)
	, hOut_(GetStdHandle(STD_OUTPUT_HANDLE))
	, backBuffer_(static_cast<size_t>(width) * height)
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleTitleA(kGameTitle);

	DWORD mode = 0;
	if (GetConsoleMode(hOut_, &mode))
	{
		mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut_, mode);
	}

	SMALL_RECT tinyRect{ 0, 0, 1, 1 };
	SetConsoleWindowInfo(hOut_, TRUE, &tinyRect);

	COORD bufferSize{ static_cast<SHORT>(width_), static_cast<SHORT>(height_) };
	SetConsoleScreenBufferSize(hOut_, bufferSize);

	SMALL_RECT windowRect{ 0, 0, static_cast<SHORT>(width_ - 1), static_cast<SHORT>(height_ - 1) };
	SetConsoleWindowInfo(hOut_, TRUE, &windowRect);

	std::ios::sync_with_stdio(false);
	out_ << "\x1b[?25l" << std::flush;

	clear(Color::Black);
}

Console::~Console()
{
	out_ << "\x1b[0m\x1b[?25h" << std::flush;
}

int Console::toAnsiForeground(Color c)
{
	static constexpr int kFgCodes[16] = {
		30, // Black
		34, // DarkBlue
		32, // DarkGreen
		36, // DarkSkyBlue
		31, // DarkRed
		35, // DarkViolet
		33, // DarkYellow
		37, // Gray
		90, // DarkGray
		94, // Blue
		92, // Green
		96, // SkyBlue
		91, // Red
		95, // Violet
		93, // Yellow
		97  // White
	};
	return kFgCodes[static_cast<unsigned short>(c) & 0x0F];
}

int Console::toAnsiBackground(Color c)
{
	static constexpr int kBgCodes[16] = {
		40,  // Black
		44,  // DarkBlue
		42,  // DarkGreen
		46,  // DarkSkyBlue
		41,  // DarkRed
		45,  // DarkViolet
		43,  // DarkYellow
		47,  // Gray
		100, // DarkGray
		104, // Blue
		102, // Green
		106, // SkyBlue
		101, // Red
		105, // Violet
		103, // Yellow
		107  // White
	};
	return kBgCodes[static_cast<unsigned short>(c) & 0x0F];
}

void Console::clear(Color bg)
{
	for (auto& cell : backBuffer_)
	{
		cell.glyph = " ";
		cell.fg = Color::Gray;
		cell.bg = bg;
		cell.isTrail = false;
	}
}

void Console::present()
{
	std::string frame;
	frame.reserve(static_cast<size_t>(width_) * height_ * 4);

	Color lastFg = Color::Gray;
	Color lastBg = Color::Black;
	frame += "\x1b[37;40m";

	for (int y = 0; y < height_; ++y)
	{
		frame += "\x1b[";
		frame += std::to_string(y + 1);
		frame += ";1H";

		const size_t rowBase = static_cast<size_t>(y) * width_;
		for (int x = 0; x < width_; ++x)
		{
			const ScreenCell& cell = backBuffer_[rowBase + x];
			if (cell.isTrail)
			{
				continue;
			}

			if (cell.fg != lastFg || cell.bg != lastBg)
			{
				frame += "\x1b[";
				frame += std::to_string(toAnsiForeground(cell.fg));
				frame += ';';
				frame += std::to_string(toAnsiBackground(cell.bg));
				frame += 'm';
				lastFg = cell.fg;
				lastBg = cell.bg;
			}

			frame += cell.glyph;
		}
	}

	out_ << frame << std::flush;
}

size_t Console::decodeUtf8(const std::string& text, size_t pos, uint32_t& outCodepoint)
{
	const unsigned char b0 = static_cast<unsigned char>(text[pos]);
	if (b0 < 0x80)
	{
		outCodepoint = b0;
		return 1;
	}
	if ((b0 & 0xE0) == 0xC0 && pos + 1 < text.size())
	{
		outCodepoint = ((b0 & 0x1F) << 6) |
			(static_cast<unsigned char>(text[pos + 1]) & 0x3F);
		return 2;
	}
	if ((b0 & 0xF0) == 0xE0 && pos + 2 < text.size())
	{
		outCodepoint = ((b0 & 0x0F) << 12) |
			((static_cast<unsigned char>(text[pos + 1]) & 0x3F) << 6) |
			(static_cast<unsigned char>(text[pos + 2]) & 0x3F);
		return 3;
	}
	if ((b0 & 0xF8) == 0xF0 && pos + 3 < text.size())
	{
		outCodepoint = ((b0 & 0x07) << 18) |
			((static_cast<unsigned char>(text[pos + 1]) & 0x3F) << 12) |
			((static_cast<unsigned char>(text[pos + 2]) & 0x3F) << 6) |
			(static_cast<unsigned char>(text[pos + 3]) & 0x3F);
		return 4;
	}

	outCodepoint = '?';
	return 1;
}

int Console::getCodepointWidth(uint32_t cp)
{
	if (cp <= 0x007F)
	{
		return 1;
	}

	// Hangul Jamo, Compatibility Jamo, Hangul Syllables
	if ((cp >= 0x1100 && cp <= 0x11FF) ||
		(cp >= 0x3130 && cp <= 0x318F) ||
		(cp >= 0xAC00 && cp <= 0xD7A3))
	{
		return 2;
	}

	// CJK Radicals / Symbols / Unified Ideographs / Fullwidth Forms
	if ((cp >= 0x2E80 && cp <= 0x9FFF) ||
		(cp >= 0xF900 && cp <= 0xFAFF) ||
		(cp >= 0xFE30 && cp <= 0xFE4F) ||
		(cp >= 0xFF01 && cp <= 0xFF60) ||
		(cp >= 0xFFE0 && cp <= 0xFFE6))
	{
		return 2;
	}

	return 1;
}

int Console::getStringWidth(const std::string& text)
{
	int w = 0;
	size_t i = 0;
	while (i < text.size())
	{
		uint32_t cp = 0;
		const size_t len = decodeUtf8(text, i, cp);
		w += getCodepointWidth(cp);
		i += len;
	}
	return w;
}

void Console::drawChar(int x, int y, char ch, Color fg, Color bg)
{
	if (x < 0 || x >= width_ || y < 0 || y >= height_)
	{
		return;
	}
	ScreenCell& cell = backBuffer_[static_cast<size_t>(y) * width_ + x];
	cell.glyph.assign(1, ch);
	cell.fg = fg;
	cell.bg = bg;
	cell.isTrail = false;
}

void Console::drawGlyph(int x, int y, const std::string& glyph, int displayWidth, Color fg, Color bg)
{
	if (y < 0 || y >= height_)
	{
		return;
	}

	if (displayWidth <= 1)
	{
		if (x < 0 || x >= width_) return;
		ScreenCell& cell = backBuffer_[static_cast<size_t>(y) * width_ + x];
		cell.glyph = glyph;
		cell.fg = fg;
		cell.bg = bg;
		cell.isTrail = false;
	}
	else
	{
		if (x < 0 || x + 1 >= width_) return;
		const size_t idx = static_cast<size_t>(y) * width_ + x;
		backBuffer_[idx].glyph = glyph;
		backBuffer_[idx].fg = fg;
		backBuffer_[idx].bg = bg;
		backBuffer_[idx].isTrail = false;

		backBuffer_[idx + 1].glyph.clear();
		backBuffer_[idx + 1].fg = fg;
		backBuffer_[idx + 1].bg = bg;
		backBuffer_[idx + 1].isTrail = true;
	}
}

void Console::drawText(int x, int y, const std::string& text, Color fg, Color bg)
{
	if (y < 0 || y >= height_)
	{
		return;
	}

	int cursorX = x;
	size_t i = 0;
	while (i < text.size())
	{
		if (text[i] == '\n')
		{
			break;
		}
		uint32_t cp = 0;
		const size_t len = decodeUtf8(text, i, cp);
		const int cw = getCodepointWidth(cp);
		if (cursorX + cw > width_)
		{
			break;
		}
		drawGlyph(cursorX, y, text.substr(i, len), cw, fg, bg);
		cursorX += cw;
		i += len;
	}
}

void Console::fillRect(int x, int y, int w, int h, char ch, Color fg, Color bg)
{
	for (int row = 0; row < h; ++row)
	{
		for (int col = 0; col < w; ++col)
		{
			drawChar(x + col, y + row, ch, fg, bg);
		}
	}
}
