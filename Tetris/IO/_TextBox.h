#pragma once

#include "_Console.h"
#include "../Core/_Types.h"
#include <string>
#include <vector>

enum class BorderStyle
{
	None,
	Single,
	Bold,
	Double
};

class TextBox
{
public:
	TextBox() = default;
	TextBox(int x, int y, int maxLineWidth, const std::string& text,
		BorderStyle borderStyle = BorderStyle::Single);
	virtual ~TextBox() = default;

	void setPosition(int x, int y);
	void setMaxLineWidth(int maxLineWidth);
	void setFixedHeight(int height);
	void setPaddingX(int paddingX);
	void setBorderStyle(BorderStyle style);
	void setColors(Color textColor, Color borderColor, Color bgColor = Color::Black, bool fillBg = true);
	void setTitle(const std::string& title);
	void setText(const std::string& text);

	int getX() const { return x_; }
	int getY() const { return y_; }
	int getMaxLineWidth() const { return maxLineWidth_; }

	static std::vector<std::string> wrapText(const std::string& text, int maxCols);

	virtual void draw(Console& console) const;

protected:
	int x_ = 0;
	int y_ = 0;
	int maxLineWidth_ = 30;
	int fixedHeight_ = 0;
	int paddingX_ = 1;
	BorderStyle borderStyle_ = BorderStyle::Single;
	Color textColor_ = Color::White;
	Color borderColor_ = Color::Gray;
	Color bgColor_ = Color::Black;
	bool fillBackground_ = true;
	std::string title_;
	std::string text_;
};
