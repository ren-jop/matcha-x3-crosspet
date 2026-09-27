#include "GamesActivity.h"

#include <I18n.h>
#include <Memory.h>

#include "CaroActivity.h"
#include "ChessActivity.h"
#include "GameScores.h"
#include "MinesweeperActivity.h"
#include "SudokuActivity.h"
#include "TwentyFortyEightActivity.h"
#include "activities/home/ReadingStatsActivity.h"
#include "components/UITheme.h"

namespace {
constexpr int kGameCount = 6;
}

void GamesActivity::onEnter() {
  Activity::onEnter();
  GAME_SCORES.loadFromFile();
  requestUpdate();
}

void GamesActivity::loop() {
  navigator.onNext([this] {
    selected = ButtonNavigator::nextIndex(selected, kGameCount);
    requestUpdate();
  });
  navigator.onPrevious([this] {
    selected = ButtonNavigator::previousIndex(selected, kGameCount);
    requestUpdate();
  });
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (!mappedInput.wasReleased(MappedInputManager::Button::Confirm)) return;

  std::unique_ptr<Activity> game;
  switch (selected) {
    case 0:
      game = makeUniqueNoThrow<ReadingStatsActivity>(renderer, mappedInput);
      break;
    case 1:
      game = makeUniqueNoThrow<TwentyFortyEightActivity>(renderer, mappedInput);
      break;
    case 2:
      game = makeUniqueNoThrow<SudokuActivity>(renderer, mappedInput);
      break;
    case 3:
      game = makeUniqueNoThrow<MinesweeperActivity>(renderer, mappedInput);
      break;
    case 4:
      game = makeUniqueNoThrow<CaroActivity>(renderer, mappedInput);
      break;
    case 5:
      game = makeUniqueNoThrow<ChessActivity>(renderer, mappedInput);
      break;
  }
  if (game)
    activityManager.pushActivity(std::move(game));
  else
    LOG_ERR("GAMES", "Not enough memory to launch game");
}

void GamesActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  const char* names[] = {tr(STR_STATS), tr(STR_2048), tr(STR_SUDOKU), tr(STR_MINESWEEPER), tr(STR_CARO), tr(STR_CHESS)};
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_GAMES));
  const int top = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  GUI.drawList(renderer, Rect{0, top, width, height - top - metrics.buttonHintsHeight}, kGameCount, selected,
               [names](int i) { return std::string(names[i]); });
  const auto hints = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, hints.btn1, hints.btn2, hints.btn3, hints.btn4);
  renderer.displayBuffer();
}
