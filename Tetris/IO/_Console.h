#pragma once

#include "../Core/_Types.h"
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

class Console
{
public:
	static constexpr int DefaultWidth = 120;
	static constexpr int DefaultHeight = 36;

	explicit Console(std::ostream& out = std::cout, int width = DefaultWidth, int height = DefaultHeight);
	~Console();

	void clear(Color bg = Color::Black);
	void present();

	void drawChar(int x, int y, char ch, Color fg = Color::Gray, Color bg = Color::Black);
	void drawGlyph(int x, int y, const std::string& glyph, int displayWidth,
		Color fg = Color::Gray, Color bg = Color::Black);
	void drawText(int x, int y, const std::string& text, Color fg = Color::Gray, Color bg = Color::Black);
	void fillRect(int x, int y, int w, int h, char ch = ' ', Color fg = Color::Gray, Color bg = Color::Black);

	int getWidth() const { return width_; }
	int getHeight() const { return height_; }

	static size_t decodeUtf8(const std::string& text, size_t pos, uint32_t& outCodepoint);
	static int getCodepointWidth(uint32_t cp);
	static int getStringWidth(const std::string& text);

private:
	struct ScreenCell
	{
		std::string glyph = " ";
		Color fg = Color::Gray;
		Color bg = Color::Black;
		bool isTrail = false;
	};

	static int toAnsiForeground(Color c);
	static int toAnsiBackground(Color c);

	std::ostream& out_;
	int width_;
	int height_;
	HANDLE hOut_;
	std::vector<ScreenCell> backBuffer_;
	bool isWindowsTerminal_ = false;
	bool showingSizeWarning_ = false;
	int lastWarnW_ = 0;
	int lastWarnH_ = 0;
};
