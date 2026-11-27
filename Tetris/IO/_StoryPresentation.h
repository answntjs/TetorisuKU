#pragma once

#include <string_view>

class Console;
enum class PursuitCue;

namespace StoryPresentation
{
	enum class Icon { Police, Suspect, Arrest, RotationLock, Badge };

	void drawPortrait(Console& console, int x, int y, int caseNumber);
	void drawIcon(Console& console, int x, int y, Icon icon, int caseNumber = 1);
	std::string_view briefing(int caseNumber, bool handoff);
	std::string_view dialogue(PursuitCue cue, int caseNumber);
	inline constexpr std::string_view FearOpening = "다들 여기까진 잘 쫓아왔어.";
	inline constexpr std::string_view Promotion = "이번 추격, 네가 끝냈어. 오늘부로 진급이다.";
}
