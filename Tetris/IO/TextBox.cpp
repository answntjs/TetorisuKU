#include "_TextBox.h"
#include <algorithm>
#include <cstdint>

TextBox::TextBox(int x, int y, int maxLineWidth, const std::string& text, BorderStyle borderStyle)
	: x_(x)
	, y_(y)
	, maxLineWidth_(std::max(2, maxLineWidth))
	, borderStyle_(borderStyle)
	, text_(text)
{
}

void TextBox::setPosition(int x, int y)
{
	x_ = x;
	y_ = y;
}

void TextBox::setMaxLineWidth(int maxLineWidth)
{
	maxLineWidth_ = std::max(2, maxLineWidth);
}

void TextBox::setFixedHeight(int height)
{
	fixedHeight_ = std::max(0, height);
}

void TextBox::setPaddingX(int paddingX)
{
	paddingX_ = std::max(0, paddingX);
}

void TextBox::setBorderStyle(BorderStyle style)
{
	borderStyle_ = style;
}

void TextBox::setColors(Color textColor, Color borderColor, Color bgColor, bool fillBg)
{
	textColor_ = textColor;
	borderColor_ = borderColor;
	bgColor_ = bgColor;
	fillBackground_ = fillBg;
}

void TextBox::setTitle(const std::string& title)
{
	title_ = title;
}

void TextBox::setText(const std::string& text)
{
	text_ = text;
}

std::vector<std::string> TextBox::wrapText(const std::string& text, int maxCols)
{
	std::vector<std::string> lines;
	if (maxCols <= 0)
	{
		return lines;
	}

	std::string current;
	int currentWidth = 0;
	size_t i = 0;

	while (i < text.size())
	{
		if (text[i] == '\r')
		{
			++i;
			continue;
		}
		if (text[i] == '\n')
		{
			lines.push_back(current);
			current.clear();
			currentWidth = 0;
			++i;
			continue;
		}

		uint32_t cp = 0;
		const size_t len = Console::decodeUtf8(text, i, cp);
		const int cw = Console::getCodepointWidth(cp);
		if (currentWidth + cw > maxCols)
		{
			lines.push_back(current);
			current.clear();
			currentWidth = 0;
		}
		current.append(text, i, len);
		currentWidth += cw;
		i += len;
	}

	lines.push_back(current);
	return lines;
}

void TextBox::draw(Console& console) const
{
	const std::vector<std::string> lines = wrapText(text_, maxLineWidth_);
	const int contentRows = std::max(1, static_cast<int>(lines.size()));
	const int innerRows = (fixedHeight_ > 0) ? fixedHeight_ : contentRows;
	const int innerWidth = maxLineWidth_ + paddingX_ * 2;

	if (borderStyle_ == BorderStyle::None)
	{
		if (fillBackground_)
		{
			console.fillRect(x_, y_, innerWidth, innerRows, ' ', textColor_, bgColor_);
		}
		for (int i = 0; i < static_cast<int>(lines.size()) && i < innerRows; ++i)
		{
			console.drawText(x_ + paddingX_, y_ + i, lines[i], textColor_, bgColor_);
		}
		return;
	}

	char tl = '+', tr = '+', bl = '+', br = '+', h = '-', v = '|';
	if (borderStyle_ == BorderStyle::Bold)
	{
		tl = '#'; tr = '#'; bl = '#'; br = '#'; h = '='; v = '#';
	}
	else if (borderStyle_ == BorderStyle::Double)
	{
		tl = '+'; tr = '+'; bl = '+'; br = '+'; h = '='; v = '|';
	}

	const int totalWidth = innerWidth + 2;
	const int totalHeight = innerRows + 2;

	if (fillBackground_)
	{
		console.fillRect(x_, y_, totalWidth, totalHeight, ' ', textColor_, bgColor_);
	}

	console.drawChar(x_, y_, tl, borderColor_, bgColor_);
	console.drawChar(x_, y_ + totalHeight - 1, bl, borderColor_, bgColor_);
	for (int cx = 1; cx <= innerWidth; ++cx)
	{
		console.drawChar(x_ + cx, y_, h, borderColor_, bgColor_);
		console.drawChar(x_ + cx, y_ + totalHeight - 1, h, borderColor_, bgColor_);
	}
	console.drawChar(x_ + 1 + innerWidth, y_, tr, borderColor_, bgColor_);
	console.drawChar(x_ + 1 + innerWidth, y_ + totalHeight - 1, br, borderColor_, bgColor_);

	for (int r = 1; r <= innerRows; ++r)
	{
		console.drawChar(x_, y_ + r, v, borderColor_, bgColor_);
		console.drawChar(x_ + 1 + innerWidth, y_ + r, v, borderColor_, bgColor_);
	}

	if (!title_.empty())
	{
		const int titleW = Console::getStringWidth(title_);
		if (titleW + 2 <= innerWidth)
		{
			console.drawText(x_ + 2, y_, " " + title_ + " ", borderColor_, bgColor_);
		}
	}

	for (int i = 0; i < static_cast<int>(lines.size()) && i < innerRows; ++i)
	{
		console.drawText(x_ + 1 + paddingX_, y_ + 1 + i, lines[i], textColor_, bgColor_);
	}
}
