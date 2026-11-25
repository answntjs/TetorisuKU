#include "../Core/_Game.h"
#include "../IO/_TextBox.h"
#include "_PlayScene.h"
#include "_TitleScene.h"
#include <algorithm>

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
  if (input.isKeyPressed(VK_UP, true) || input.isKeyPressed('W', true)) {
    selectedMode_ = (selectedMode_ + 3) % 4;
  } else if (input.isKeyPressed(VK_DOWN, true) ||
             input.isKeyPressed('S', true)) {
    selectedMode_ = (selectedMode_ + 1) % 4;
  } else if (input.isKeyPressed(VK_LEFT, true) ||
             input.isKeyPressed('A', true)) {
    startLevel_ = std::max(1, startLevel_ - 1);
  } else if (input.isKeyPressed(VK_RIGHT, true) ||
             input.isKeyPressed('D', true)) {
    startLevel_ = std::min(8, startLevel_ + 1);
  } else if (input.isKeyPressed('B')) {
    borderless_ = !borderless_;
  } else if (input.isKeyPressed(VK_RETURN) || input.isKeyPressed(VK_SPACE)) {
    GameMode mode = GameMode::Single;
    switch (selectedMode_) {
    case 0:
      mode = GameMode::Single;
      break;
    case 1:
      mode = GameMode::PvP;
      break;
    case 2:
      mode = GameMode::VsBot;
      break;
    case 3:
      mode = GameMode::PvE;
      break;
    }
    game_.changeScene(
        std::make_unique<PlayScene>(game_, mode, startLevel_, borderless_));
  } else if (input.isKeyPressed(VK_ESCAPE)) {
    game_.quit();
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

  const char *modeNames[4] = {
      "1. SINGLE PLAY (Original Stages)", "2. PvP (2P Local Versus)",
      "3. Vs BOT (AI Versus)", "4. PvE (Garbage Survival)"};

  std::string menuText;
  for (int i = 0; i < 4; ++i) {
    menuText += (i == selectedMode_) ? "> " : "  ";
    menuText += modeNames[i];
    menuText += "\n";
  }
  menuText +=
      "\nStart Level (LEFT/RIGHT): " + std::to_string(startLevel_) + " / 8\n";
  menuText += std::string("Borderless (B): ") +
              (borderless_ ? "ON (No Side Walls)" : "OFF (Classic Walls)");

  TextBox menuBox(12, 11, 42, menuText, BorderStyle::Single);
  menuBox.setTitle("GAME MODE");
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

  console.drawText(38, 30, "[ENTER / SPACE] Start Game    [ESC] Exit",
                   Color::Yellow);
}
