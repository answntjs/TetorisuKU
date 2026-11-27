#pragma once

#include <array>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

struct KeyBinding
{
	int rotateCW = VK_UP;
	int rotateCCW = 'Z';
	int moveLeft = VK_LEFT;
	int moveRight = VK_RIGHT;
	int softDrop = VK_DOWN;
	int hardDrop = VK_SPACE;
	int hold = VK_LSHIFT;
	int useItem = 'E';

	static KeyBinding SingleDefault() { return {}; }

	static KeyBinding Player1Default()
	{
		return { 'W', 'Q', 'A', 'D', 'S', VK_SPACE, VK_LSHIFT, 'E' };
	}

	static KeyBinding Player2Default()
	{
		return { VK_UP, VK_OEM_2, VK_LEFT, VK_RIGHT, VK_DOWN, VK_RSHIFT, VK_RETURN, VK_RCONTROL };
	}
};

class InputHandler
{
public:
	static constexpr int KeyCount = 256;

	InputHandler();

	void update(float dt);
	void reset();

	bool isKeyDown(int vk) const;
	bool isKeyPressed(int vk, bool allowRepeat = false) const;
	bool isKeyRepeated(int vk) const;
	bool isAnyKeyPressed() const;

private:
	float systemRepeatDelay_ = 0.50f;
	float systemRepeatInterval_ = 0.033f;
	std::array<bool, KeyCount> prevKeys_{};
	std::array<bool, KeyCount> currKeys_{};
	std::array<float, KeyCount> holdTimers_{};
	std::array<bool, KeyCount> repeatTriggered_{};
	std::array<float, KeyCount> repeatTimers_{};
};
