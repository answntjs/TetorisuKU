#pragma once

#include <memory>
#include <vector>

inline constexpr const char *kGameTitle = "TetorisuKU(가제)"; // 제목

class Player;
class GameManager;
class IBlockEffect;

enum class Color : unsigned short {
  Black = 0,
  DarkBlue = 1,
  DarkGreen = 2,
  DarkSkyBlue = 3,
  DarkRed = 4,
  DarkViolet = 5,
  DarkYellow = 6,
  Gray = 7,
  DarkGray = 8,
  Blue = 9,
  Green = 10,
  SkyBlue = 11,
  Red = 12,
  Violet = 13,
  Yellow = 14,
  White = 15
};

enum class BlockType : int {
  I = 0,
  O = 1,
  T = 2,
  J = 3,
  L = 4,
  S = 5,
  Z = 6,
  Count = 7
};

enum class GameMode { Single, PvP, VsBot, PvE, StorySolo, StoryDuel };

constexpr bool isStoryMode(GameMode mode)
{
  return mode == GameMode::StorySolo || mode == GameMode::StoryDuel;
}

struct Viewport {
  int originX = 0;
  int originY = 0;
};

struct ClearedBlockEffect {
  std::shared_ptr<IBlockEffect> effect;
  int row;
  int col;
};

struct LineClearResult {
  int clearedLines = 0;
  int lineClearScore = 0;
  int comboCount = 0;
  bool isTSpin = false;
  bool isBackToBack = false;
  std::vector<ClearedBlockEffect> triggeredEffects;
};
