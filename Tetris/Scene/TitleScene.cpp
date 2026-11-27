#include "../Core/_Game.h"
#include "../Domain/_PursuitStage.h"
#include "../IO/_TextBox.h"
#include "_PlayScene.h"
#include "_TitleScene.h"
#include <algorithm>
#include <array>

namespace {
constexpr std::array<GameMode, 2> kStoryModes{GameMode::StorySolo,
                                              GameMode::StoryDuel};
constexpr std::array<GameMode, 4> kClassicModes{GameMode::Single, GameMode::PvP,
                                                GameMode::VsBot, GameMode::PvE};
} // namespace

TitleScene::TitleScene(Game &game) : Scene(game), rng_(std::random_device{}()) {
  randomizePreviewBlocks();
}

void TitleScene::onEnter() {
  animTimer_ = 0.0f;
  confirmExit_ = false;
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
  if (confirmExit_) {
    if (input.isKeyPressed(VK_RETURN)) {
      game_.quit();
    } else if (input.isKeyPressed(VK_ESCAPE)) {
      confirmExit_ = false;
    }
    return;
  }

  const int count =
      menu_ == Menu::Classic ? static_cast<int>(kClassicModes.size()) : 2;
  if (input.isKeyPressed(VK_UP, true) || input.isKeyPressed('W', true)) {
    selection_ = (selection_ + count - 1) % count;
  } else if (input.isKeyPressed(VK_DOWN, true) ||
             input.isKeyPressed('S', true)) {
    selection_ = (selection_ + 1) % count;
  } else if (input.isKeyPressed(VK_ESCAPE)) {
    if (menu_ == Menu::Main) {
      confirmExit_ = true;
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
      const auto mode =
          story ? kStoryModes[selection_] : kClassicModes[selection_];
      const int level = mode == GameMode::StorySolo ? storyStartLevel_
                        : mode == GameMode::VsBot   ? botLevel_
                        : story                     ? 1
                                                    : startLevel_;
      game_.changeScene(std::make_unique<PlayScene>(game_, mode, level,
                                                    !story && borderless_));
    }
  } else if (menu_ == Menu::Classic && input.isKeyPressed('B')) {
    borderless_ = !borderless_;
  } else if (menu_ != Menu::Main &&
             (selection_ == 0 || (menu_ == Menu::Classic && selection_ == 2))) {
    const bool story = menu_ == Menu::Story;
    const bool isBot = (menu_ == Menu::Classic && selection_ == 2);
    int &level = story ? storyStartLevel_ : isBot ? botLevel_ : startLevel_;
    const int lastLevel = story ? StoryCaseCount : isBot ? 5 : 8;
    if (input.isKeyPressed(VK_LEFT, true) || input.isKeyPressed('A', true)) {
      level = std::max(1, level - 1);
    } else if (input.isKeyPressed(VK_RIGHT, true) ||
               input.isKeyPressed('D', true)) {
      level = std::min(lastLevel, level + 1);
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
                  BorderStyle::Bold); // 게임 제목 정하기!!!
  logoBox.setTitle(kGameTitle);
  logoBox.setColors(Color::SkyBlue, Color::Yellow, Color::Black, true);
  logoBox.draw(console);

  std::string menuText;
  const auto addOptions = [&](const auto &names) {
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
    if (selection_ == 0) {
      menuText += "\n시작 사건 (좌우)  " + std::to_string(storyStartLevel_) +
                  " / " + std::to_string(StoryCaseCount) + "  " +
                  getStoryCase(storyStartLevel_).title;
    }
  } else {
    addOptions(std::array{"싱글 플레이", "로컬 2P", "봇 대전", "방해 줄 생존"});
    if (selection_ == 0) {
      menuText +=
          "\n시작 난이도 (좌우)  " + std::to_string(startLevel_) + " / 8";
    } else if (selection_ == 2) {
      menuText += "\n봇 난이도 (좌우)  " + std::to_string(botLevel_) + " / 5" +
                  (botLevel_ == 5 ? " (점진 가속)" : "");
    }
    menuText += borderless_ ? "\n[B] 좌우 연결 켜짐" : "\n[B] 좌우 연결 꺼짐";
  }

  TextBox menuBox(12, 11, 42, menuText, BorderStyle::Single);
  menuBox.setTitle(menu_ == Menu::Story     ? "스토리 모드"
                   : menu_ == Menu::Classic ? "일반 모드"
                                            : "모드 선택");
  menuBox.setColors(Color::White, Color::Gray, Color::Black, true);
  menuBox.draw(console);

  TextBox keyBox(60, 11, 46,
                 "[싱글 / 봇 대전 / 생존 / 1인 수사]\n"
                 "  이동: 좌/우  회전: 위/Z\n"
                 "  드롭: 아래/SPACE  홀드: LSHIFT  정지: ESC\n"
                 "[2인 대전 / 2인 경찰과 도둑]\n"
                 "  1P: WASD + SPACE(하드) + LSHIFT(홀드)\n"
                 "  2P: 방향키 + RSHIFT(하드) + ENTER(홀드)",
                 BorderStyle::Single);
  keyBox.setTitle("조작키");
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

  if (confirmExit_) {
    TextBox exitBox(41, 13, 34,
                    "게임을 종료하시겠습니까?\n\n"
                    "  [ENTER] 종료\n"
                    "  [ESC]   취소",
                    BorderStyle::Bold);
    exitBox.setTitle("종료 확인");
    exitBox.setColors(Color::White, Color::White, Color::DarkBlue, true);
    exitBox.draw(console);
  }
}
