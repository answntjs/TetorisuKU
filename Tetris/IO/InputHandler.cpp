#include "_InputHandler.h"

InputHandler::InputHandler()
{
	DWORD delayVal = 1;
	if (SystemParametersInfoW(SPI_GETKEYBOARDDELAY, 0, &delayVal, 0))
	{
		// 0..3 -> 250ms, 500ms, 750ms, 1000ms
		systemRepeatDelay_ = 0.25f * static_cast<float>(delayVal + 1);
	}

	DWORD speedVal = 31;
	if (SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0, &speedVal, 0))
	{
		// 0..31 -> ~2.5 to ~30 repeats/sec
		const float repeatsPerSec = 2.5f + (static_cast<float>(speedVal) / 31.0f) * 27.5f;
		systemRepeatInterval_ = 1.0f / repeatsPerSec;
	}

	reset();
}

void InputHandler::reset()
{
	prevKeys_.fill(false);
	currKeys_.fill(false);
	holdTimers_.fill(0.0f);
	repeatTriggered_.fill(false);
	repeatTimers_.fill(0.0f);
}

void InputHandler::update(float dt)
{
	prevKeys_ = currKeys_;
	repeatTriggered_.fill(false);

	for (int vk = 0; vk < KeyCount; ++vk)
	{
		const bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
		currKeys_[vk] = down;

		if (down)
		{
			if (!prevKeys_[vk])
			{
				holdTimers_[vk] = 0.0f;
				repeatTimers_[vk] = 0.0f;
				repeatTriggered_[vk] = true;
			}
			else
			{
				holdTimers_[vk] += dt;
				if (holdTimers_[vk] >= systemRepeatDelay_)
				{
					repeatTimers_[vk] += dt;
					if (repeatTimers_[vk] >= systemRepeatInterval_)
					{
						repeatTimers_[vk] -= systemRepeatInterval_;
						repeatTriggered_[vk] = true;
					}
				}
			}
		}
		else
		{
			holdTimers_[vk] = 0.0f;
			repeatTimers_[vk] = 0.0f;
		}
	}
}

bool InputHandler::isKeyDown(int vk) const
{
	if (vk < 0 || vk >= KeyCount) return false;
	return currKeys_[vk];
}

bool InputHandler::isKeyPressed(int vk, bool allowRepeat) const
{
	if (vk < 0 || vk >= KeyCount) return false;
	if (allowRepeat)
	{
		return repeatTriggered_[vk];
	}
	return currKeys_[vk] && !prevKeys_[vk];
}

bool InputHandler::isKeyRepeated(int vk) const
{
	if (vk < 0 || vk >= KeyCount) return false;
	return repeatTriggered_[vk];
}

bool InputHandler::isAnyKeyPressed() const
{
	// Skip mouse buttons (1..6) and modifier-only noise if desired; check standard keyboard keys
	for (int vk = 0x08; vk < KeyCount; ++vk)
	{
		if (vk == VK_LSHIFT || vk == VK_RSHIFT || vk == VK_SHIFT ||
			vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL ||
			vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU)
		{
			continue;
		}
		if (currKeys_[vk] && !prevKeys_[vk])
		{
			return true;
		}
	}
	return false;
}
