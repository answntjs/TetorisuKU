#include "../Core/_Game.h"
#include "../IO/_TextBox.h"
#include "_PlayScene.h"
#include "_TitleScene.h"
#include <algorithm>

namespace {
constexpr std::array<GameMode, 2> kStoryModes{GameMode::StorySolo, GameMode::StoryDuel};
constexpr std::array<GameMode, 4> kClassicModes{GameMode::Single, GameMode::PvP,
                                                  GameMode::VsBot, GameMode::PvE};
}

TitleScene::TitleScene(Game &game) : Scene(game), rng_(std::random_device{}()) {
  randomizePreviewBlocks();
}

void TitleScene::onEnter() {
  animTimer_ = 0.0f;
  randomizePreviewBlocks();
}

void TitleScene::randomizePreviewBlocks() {
  std::uniform_int_distribution<int> shapeDist(
      0, static_cast<int>(BlockType::Count) - 1);
  std::uniform_int_distribution<int> angleDist(0, 3);
  for (auto &b : previewBlocks_) {
    const auto shape = static_cast<BlockType>(shapeDist(rng_));
    b = Block(shape, Block::getDefaultColor(shape));
    b.setAngle(angleDist(rng_));
  }
}

void TitleScene::handleInput(const InputHandler &input) {
  const int count = menu_ == Menu::Classic ? static_cast<int>(kClassicModes.size()) : 2;
  if (input.isKeyPressed(VK_UP, true) || input.isKeyPressed('W', true)) {
    selection_ = (selection_ + count - 1) % count;
  } else if (input.isKeyPressed(VK_DOWN, true) || input.isKeyPressed('S', true)) {
    selection_ = (selection_ + 1) % count;
  } else if (input.isKeyPressed(VK_ESCAPE)) {
    if (menu_ == Menu::Main) {
      game_.quit();
    } else {
      selection_ = menu_ == Menu::Story ? 0 : 1;
      menu_ = Menu::Main;
    }
  } else if (input.isKeyPressed(VK_RETURN) || input.isKeyPressed(VK_SPACE)) {
    if (menu_ == Menu::Main) {
      menu_ = selection_ == 0 ? Menu::Story : Menu::Classic;
      selection_ = 0;
    } else {
      const bool story = menu_ == Menu::Story;
      const auto mode = story ? kStoryModes[selection_] : kClassicModes[selection_];
      game_.changeScene(std::make_unique<PlayScene>(
          game_, mode, story ? 1 : startLevel_, !story && borderless_));
    }
  } else if (menu_ == Menu::Classic) {
    if (input.isKeyPressed('B')) {
      borderless_ = !borderless_;
    } else if (selection_ == 0 && (input.isKeyPressed(VK_LEFT, true) || input.isKeyPressed('A', true))) {
      startLevel_ = std::max(1, startLevel_ - 1);
    } else if (selection_ == 0 && (input.isKeyPressed(VK_RIGHT, true) || input.isKeyPressed('D', true))) {
      startLevel_ = std::min(8, startLevel_ + 1);
    }
  }
}

void TitleScene::update(float dt) {
  animTimer_ += dt;
  if (animTimer_ >= 1.2f) {
    animTimer_ -= 1.2f;
    randomizePreviewBlocks();
  }
}

void TitleScene::render(Console &console, const Renderer &) const {
  TextBox logoBox(28, 2, 58,
                  "#####  #####  #####  ####   ###   #### \n"
                  "  #    #        #    #   #   #   #     \n"
                  "  #    ####     #    ####    #    ###  \n"
                  "  #    #        #    #  #    #       # \n"
                  "  #    #####    #    #   #  ###  ####  ",
                  BorderStyle::Bold);
  logoBox.setTitle(kGameTitle);
  logoBox.setColors(Color::SkyBlue, Color::Yellow, Color::Black, true);
  logoBox.draw(console);

  std::string menuText;
  const auto addOptions = [&](const auto& names) {
    for (size_t i = 0; i < names.size(); ++i) {
      menuText += static_cast<int>(i) == selection_ ? "> " : "  ";
      menuText += names[i];
      menuText += "\n";
    }
  };
  if (menu_ == Menu::Main) {
    addOptions(std::array{"스토리 모드", "일반 모드"});
  } else if (menu_ == Menu::Story) {
    addOptions(std::array{"1인 수사", "2인 경찰과 도둑"});
  } else {
    addOptions(std::array{"싱글 플레이", "로컬 2P", "봇 대전", "방해 줄 생존"});
    if (selection_ == 0) {
      menuText += "\n시작 난이도 (좌우)  " + std::to_string(startLevel_) + " / 8";
    }
    menuText += borderless_ ? "\n[B] 좌우 연결 켜짐" : "\n[B] 좌우 연결 꺼짐";
  }

  TextBox menuBox(12, 11, 42, menuText, BorderStyle::Single);
  menuBox.setTitle(menu_ == Menu::Story ? "스토리 모드" : menu_ == Menu::Classic ? "일반 모드" : "모드 선택");
  menuBox.setColors(Color::White, Color::Gray, Color::Black, true);
  menuBox.draw(console);

  TextBox keyBox(60, 11, 46,
                 "[Single / Vs Bot / PvE]\n"
                 "  Move: LEFT/RIGHT  Rotate: UP/Z\n"
                 "  Drop: DOWN/SPACE  Hold: LSHIFT  Pause: ESC\n"
                 "[PvP 2P]\n"
                 "  1P: WASD + SPACE(Hard) + LSHIFT(Hold)\n"
                 "  2P: Arrows + RSHIFT(Hard) + ENTER(Hold)",
                 BorderStyle::Single);
  keyBox.setTitle("CONTROLS");
  keyBox.setColors(Color::Gray, Color::DarkSkyBlue, Color::Black, true);
  keyBox.draw(console);

  const int blockXs[4] = {26, 44, 62, 80};
  for (int i = 0; i < 4; ++i) {
    const auto cells = previewBlocks_[i].getLocalCells();
    for (const auto &[cx, cy] : cells) {
      console.drawText(blockXs[i] + cx * 2, 23 + cy, "[]",
                       previewBlocks_[i].getColor());
    }
  }

  console.drawText(36, 30,
                   menu_ == Menu::Main ? "[ENTER / SPACE] 선택    [ESC] 종료"
                                       : "[ENTER / SPACE] 시작    [ESC] 뒤로",
                   Color::Yellow);
}
