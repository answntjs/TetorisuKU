#pragma once

#include "../Domain/_Block.h"
#include "_Scene.h"
#include "../Core/_Types.h"
#include <array>
#include <random>

class TitleScene : public Scene {
public:
  explicit TitleScene(Game &game);

  void onEnter() override;
  void handleInput(const InputHandler &input) override;
  void update(float dt) override;
  void render(Console &console, const Renderer &renderer) const override;

private:
  void randomizePreviewBlocks();

  int selectedMode_ = 0; // 0: Single, 1: PvP, 2: VsBot, 3: PvE
  int startLevel_ = 1;
  bool borderless_ = false;

  float animTimer_ = 0.0f;
  std::mt19937
      rng_; // Mersenne Twister 19937 Pseudo-Random Generator. 보고서에 명시
  std::array<Block, 4> previewBlocks_;
};
