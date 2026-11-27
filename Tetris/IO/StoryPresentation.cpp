#include "_StoryPresentation.h"
#include "_Console.h"
#include "../Domain/_PursuitStage.h"
#include <algorithm>
#include <array>

namespace
{
	using Portrait = std::array<std::string_view, 16>;
	using SmallSprite = std::array<std::string_view, 6>;

	constexpr std::array<Portrait, StoryCaseCount + 1> kPortraits{{
		// 경찰
		{{
			"................",
			".....BBBBBB.....",
			"....BUUUUUUB....",
			"...BUUUYOUUUB...",
			"...BBBBBBBBBB...",
			"..KKKKKKKKKKKK..",
			"....KSSSSSK.....",
			"....KSKSKSK.....",
			"....KSSSSSK.....",
			".....sSSSs......",
			".....KKKKK......",
			"...BBUWWUBB.....",
			"..BBBUWWUBBB....",
			".BBBBBBYBBBBB...",
			".BBBBBBBUBBBB...",
			"..BBBBBBBBBB...."
		}},
		// 경범죄
		{{
			"................",
			"....rrrrr.......",
			"...rRRRRrr......",
			"..rRROORRRr.....",
			"..RRRRRRRrrr....",
			"..KKKKKKKKKKKK..",
			"...KSSSSSs......",
			"...KSsKSSKs.....",
			"...KSSSSSSK.....",
			"....sSSSK.......",
			"....rsssr.......",
			"..rRRwwRRr......",
			".rRrRWWRRRr.....",
			".rrRrWWrRRrr....",
			"..rrrrKrrrrr....",
			"................"
		}},
		// 소매치기
		{{
			"................",
			"......ggg.......",
			"....gGggGg......",
			"...gGGggGGg.....",
			"..gGGKKKKGGg....",
			"..gGKSsSsKGg....",
			"..gGKsKSKKGg....",
			"...gKKKKKKg..W..",
			"....gKKKKg..W.W.",
			".....gKKg..WWW..",
			"...gGgGGGg.WGw..",
			"..gGGgGGgggGGw..",
			"..gGGGGGgggg....",
			"...ggggggg......",
			"....KKKKK.......",
			"................"
		}},
		// 사기
		{{
			"................",
			".....KKKKK......",
			".....KVVVK......",
			".....KVKVK......",
			".....KvvvK......",
			"...KKKKKKKKK....",
			".....SSSSS......",
			".....KsKOSs.....",
			".....SSsOws.....",
			"......sKss......",
			".....vWwVv......",
			"...vVvWOVVv.....",
			"..vVVVWOVVVv....",
			"..vVvVwovVvV....",
			"...vvvvvvvv.....",
			"................"
		}},
		// 강도
		{{
			"................",
			"......KKK.......",
			"....KKKKKKK.....",
			"...KKKKgKKKK....",
			"...KKYYYYYKK....",
			"...KYOYYOYKK....",
			"...KYKYYKYKK....",
			"....YYOOYYK.....",
			".....OoOOK......",
			"...gGGKKGGg.....",
			"..gGGGGGGGGg....",
			".gGGoGGGGoGGg...",
			"gGGYYGGGYYGGGg..",
			".ggggGGGGgggg...",
			"..KKKKKKKKKK....",
			"................"
		}},
		// 살인 사건
		{{
			".........vv.....",
			"........vKVv....",
			"......vvKKVK....",
			".....vKKKKVKv...",
			"....vKvKKKKKKv..",
			"....vKWwKKKKKv..",
			"...vKKKKKKKvv...",
			"...vKKKKKKKv....",
			"..vKKKKKKKv.....",
			"..vKvKKKKKKv....",
			".vKKvvKKKKKv....",
			".vKKKVvKKKKKv...",
			"vKKKKKVvKKKKKv..",
			"vvKKKKKVvKKKKKv.",
			"..vvvvvvvvvvvv..",
			"................"
		}}
	}};

	constexpr std::array<SmallSprite, StoryCaseCount> kSuspects{{
		{{ "..rR..", ".rRRr.", "KKKKKK", ".KSKs.", ".rSSr.", ".rrRr." }},
		{{ "..gg..", ".gGGg.", ".gSKg.", ".gKKgW", ".gGGWW", "..ggg." }},
		{{ "..KvK.", "..KvK.", ".KKKKK", "..SsO.", ".vWVv.", ".vvVv." }},
		{{ "..KK..", ".KYYK.", ".YKYY.", "..Oo..", "gGoOGg", "ggGGgg" }},
		{{ "...vv.", "..vKKv", ".vWKKv", ".vKKv.", "vKKKv.", "vKKKKv" }}
	}};

	constexpr SmallSprite kPolice{{ "..BB..", ".BUUB.", "KKKKKK", ".KSKs.", ".BYYB.", ".BBBB." }};
	constexpr SmallSprite kCuffs{{ "......", ".WW.WW", "W..W.W", "W.W..W", ".W.WW.", "......" }};
	constexpr SmallSprite kLock{{ "..YY..", ".Y..Y.", ".Y..Y.", ".YYYY.", ".YKYY.", ".YYYY." }};
	constexpr SmallSprite kBadge{{ "..WW..", ".WYYW.", "WYYYYW", "WYOYYW", ".WYYW.", "..WW.." }};

	struct CaseBriefing
	{
		std::string_view opening;
		std::string_view handoff;
	};

	constexpr std::array<CaseBriefing, StoryCaseCount> kBriefings{{
		{
			"팀장  \"첫 출동이지? 상가 뒤 골목에 소란 신고가 들어왔어. 가서 무슨 일인지 확인해.\"\n"
			"형사  \"네. 제가 다녀오겠습니다.\"",
			""
		},
		{
			"팀장  \"시장에 소매치기 신고가 잇따라 들어왔어. 현장에 가서 확인해.\"",
			"팀장  \"첫 출동치고 침착했어. 내일부터 시장 순찰에 들어가. 요 며칠 소매치기 신고가 계속 들어와.\""
		},
		{
			"팀장  \"사기 신고가 들어왔는데 피해자들 말이 다 달라. 네가 처음부터 다시 들어봐.\"",
			"팀장  \"그 인파 속에서도 잘 잡았어, 이번엔 사기 신고인데, 피해자들 말이 다 달라. 네가 처음부터 다시 들어봐.\""
		},
		{
			"팀장  \"강도 현장에 인질이 있어. 네가 먼저 상황을 파악해.\"",
			"팀장  \"다들 같은 얘길 믿을 때 너만 다시 확인했지. 이번 현장엔 인질이 있어. 네가 먼저 상황을 파악해.\""
		},
		{
			"팀장  \"살인 사건 용의자가 모습을 드러냈다. 이번 추격은 네가 지휘해.\"",
			"팀장  \"강도 용의자는 인계했어. 살인 사건 용의자가 모습을 드러냈다. 이번 추격은 네가 지휘해.\""
		}
	}};

	Color pixelColor(char pixel)
	{
		switch (pixel)
		{
		case 'K': return Color::Black;
		case 'S': return Color::Yellow;
		case 's': return Color::DarkYellow;
		case 'w': return Color::Gray;
		case 'W': return Color::White;
		case 'G': return Color::Gray;
		case 'g': return Color::DarkGray;
		case 'Y': return Color::Yellow;
		case 'O': return Color::DarkYellow;
		case 'o': return Color::DarkRed;
		case 'B': return Color::Blue;
		case 'U': return Color::SkyBlue;
		case 'V': return Color::Violet;
		case 'v': return Color::DarkViolet;
		case 'R': return Color::Red;
		case 'r': return Color::DarkRed;
		default: return Color::Black;
		}
	}

	template<std::size_t Height>
	void drawSprite(Console& console, int x, int y, const std::array<std::string_view, Height>& pixels)
	{
		static_assert(Height % 2 == 0);
		for (std::size_t row = 0; row < Height; row += 2)
		{
			for (std::size_t col = 0; col < pixels[row].size(); ++col)
			{
				const char top = pixels[row][col];
				const char bottom = pixels[row + 1][col];
				if (top != '.' || bottom != '.')
				{
					console.drawGlyph(x + static_cast<int>(col), y + static_cast<int>(row / 2),
						"▀", 1, pixelColor(top), pixelColor(bottom));
				}
			}
		}
	}
}

void StoryPresentation::drawPortrait(Console& console, int x, int y, int caseNumber)
{
	drawSprite(console, x, y, kPortraits[std::clamp(caseNumber, 0, StoryCaseCount)]);
}

void StoryPresentation::drawIcon(Console& console, int x, int y, Icon icon, int caseNumber)
{
	switch (icon)
	{
	case Icon::Police:
		drawSprite(console, x, y, kPolice);
		break;
	case Icon::Suspect:
		drawSprite(console, x, y, kSuspects[std::clamp(caseNumber, 1, StoryCaseCount) - 1]);
		break;
	case Icon::Arrest:
		drawSprite(console, x, y, kCuffs);
		break;
	case Icon::RotationLock:
		drawSprite(console, x, y, kLock);
		break;
	case Icon::Badge:
		drawSprite(console, x, y, kBadge);
		break;
	}
}

std::string_view StoryPresentation::briefing(int caseNumber, bool handoff)
{
	const auto& brief = kBriefings[std::clamp(caseNumber, 1, StoryCaseCount) - 1];
	return handoff && !brief.handoff.empty() ? brief.handoff : brief.opening;
}

std::string_view StoryPresentation::dialogue(PursuitCue cue, int caseNumber)
{
	const bool copycat = caseNumber == StoryCaseCount;
	switch (cue)
	{
	case PursuitCue::HoldStolen:
		return copycat ? "그 소매치기도 이렇게 했던가. 이거 가져갈게."
			: "아까 빼 둔 거? 내가 챙겼어.";
	case PursuitCue::FalsePreview:
		return copycat ? "그 사기꾼 말이 맞네. 보여 주기만 해도 믿잖아."
			: "사람들은 눈에 보이는 걸 전부 믿곤 하죠.";
	case PursuitCue::HostageTaken:
		return copycat ? "그 강도가 뭐라고 했더라. 저 사람 두고도 날 쫓을 거냐고."
			: "나 계속 쫓아올 거야? 저 사람 두고?";
	case PursuitCue::RotationLocked:
		return "그거 내가 좀 손봤어. 돌려 봐야 소용없어.";
	case PursuitCue::FearWarning:
		return FearOpening;
	case PursuitCue::FearRise:
		return "…공포가 차오르기 전까진.";
	case PursuitCue::HostageExpired:
		return "구출 시간 초과. 추격 점수 감소!";
	case PursuitCue::None:
		return {};
	}
	return {};
}
