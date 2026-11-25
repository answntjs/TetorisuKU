#pragma once

#include <array>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

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
