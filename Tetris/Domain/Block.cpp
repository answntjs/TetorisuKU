#include "_Block.h"

namespace {
// SRS 4x4 shapes: [shape][angle][row][col]
constexpr int kShapes[7][4][4][4] = {
    // I
    {{{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}}},
    // O
    {{{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
    // T
    {{{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},
    // J
    {{{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 1, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}}},
    // L
    {{{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
     {{1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},
    // S
    {{{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
     {{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},
    // Z
    {{{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 1, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}}}};
} // namespace

Block::Block(BlockType shape, Color color,
             std::shared_ptr<IBlockEffect> destroyEffect)
    : shape_(shape), color_(color), destroyEffect_(std::move(destroyEffect)) {}

void Block::block_start(int spawnX, int spawnY) {
  pos_ = {spawnX, spawnY};
  angle_ = 0;
  lastActionWasRotate_ = false;
}

void Block::rotate_block(bool clockwise) {
  angle_ = clockwise ? ((angle_ + 1) % 4) : ((angle_ + 3) % 4);
  lastActionWasRotate_ = true;
}

std::array<std::pair<int, int>, 4> Block::getLocalCells(int testAngle) const {
  const int a = (testAngle < 0) ? angle_ : ((testAngle % 4) + 4) % 4;
  const int s = static_cast<int>(shape_);
  std::array<std::pair<int, int>, 4> cells{};
  int idx = 0;
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      if (kShapes[s][a][r][c] == 1 && idx < 4) {
        cells[idx++] = {c, r};
      }
    }
  }
  return cells;
}

std::array<std::pair<int, int>, 4> Block::getWorldCells(int dx, int dy,
                                                        int testAngle) const {
  auto cells = getLocalCells(testAngle);
  for (auto &cell : cells) {
    cell.first += pos_.first + dx;
    cell.second += pos_.second + dy;
  }
  return cells;
}

std::pair<int, int> Block::getPivotWorld() const {
  return {pos_.first + 1, pos_.second + 1};
}

Color Block::getDefaultColor(BlockType shape) {
  switch (shape) {
  case BlockType::I:
    return Color::SkyBlue;
  case BlockType::O:
    return Color::Yellow;
  case BlockType::T:
    return Color::Violet;
  case BlockType::J:
    return Color::Blue;
  case BlockType::L:
    return Color::DarkYellow;
  case BlockType::S:
    return Color::Green;
  case BlockType::Z:
    return Color::Red;
  default:
    return Color::White;
  }
}
