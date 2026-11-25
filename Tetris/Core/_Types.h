#pragma once

#include <memory>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX // Windows.h에서의 max, min / std::max, std::min 사용시
                 // 충돌방지
#endif
#include <Windows.h>

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

enum class GameMode { Single, PvP, VsBot, PvE };

struct KeyBinding {
  int rotateCW = VK_UP; // CW means Clockwise
  int rotateCCW = 'Z';  // CCW means Counter-Clockwise
  int moveLeft = VK_LEFT;
  int moveRight = VK_RIGHT;
  int softDrop = VK_DOWN;
  int hardDrop = VK_SPACE;
  int hold = VK_LSHIFT;
  int useItem = 'E';

  static KeyBinding SingleDefault() {
    return {VK_UP, 'Z', VK_LEFT, VK_RIGHT, VK_DOWN, VK_SPACE, VK_LSHIFT, 'E'};
  }

  static KeyBinding Player1Default() {
    return {'W', 'Q', 'A', 'D', 'S', VK_SPACE, VK_LSHIFT, 'E'};
  }

  static KeyBinding Player2Default() {
    return {VK_UP,   VK_OEM_2,  VK_LEFT,   VK_RIGHT,
            VK_DOWN, VK_RSHIFT, VK_RETURN, VK_RCONTROL};
  }
};

struct Viewport {
  int originX = 0;
  int originY = 0;
};

struct LineClearResult {
  int clearedLines = 0;
  int comboCount = 0;
  bool isTSpin = false;
  bool isBackToBack = false;
  std::vector<std::shared_ptr<IBlockEffect>> triggeredEffects;
};
