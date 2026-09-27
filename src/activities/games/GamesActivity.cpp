#include "GamesActivity.h"

#include <I18n.h>
#include <Memory.h>

#include <cstdio>

#include "CaroActivity.h"
#include "ChessActivity.h"
#include "GameScores.h"
#include "MinesweeperActivity.h"
#include "ReadingStatsStore.h"
#include "SudokuActivity.h"
#include "TwentyFortyEightActivity.h"
#include "VirtualPetActivity.h"
#include "activities/home/ReadingStatsActivity.h"
#include "components/StatsWidgets.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "pet/PetManager.h"

namespace {
constexpr int kGameCount = 7;
}

void GamesActivity::onEnter() {
  Activity::onEnter();
  GAME_SCORES.loadFromFile();
  READING_STATS_STORE.loadFromFile();
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
      game = makeUniqueNoThrow<VirtualPetActivity>(renderer, mappedInput);
      break;
    case 2:
      game = makeUniqueNoThrow<TwentyFortyEightActivity>(renderer, mappedInput);
      break;
    case 3:
      game = makeUniqueNoThrow<SudokuActivity>(renderer, mappedInput);
      break;
    case 4:
      game = makeUniqueNoThrow<MinesweeperActivity>(renderer, mappedInput);
      break;
    case 5:
      game = makeUniqueNoThrow<CaroActivity>(renderer, mappedInput);
      break;
    case 6:
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
  const char* names[] = {tr(STR_STATS),       tr(STR_VIRTUAL_PET), tr(STR_2048), tr(STR_SUDOKU),
                         tr(STR_MINESWEEPER), tr(STR_CARO),        tr(STR_CHESS)};
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, tr(STR_GAMES));
  const int bannerTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const auto today = StatsWidgets::getToday();
  const unsigned minutes = READING_STATS_STORE.getMinutesForDay(today.year, today.month, today.day);
  const auto& pet = PET_MANAGER.getState();
  const unsigned streak = PET_MANAGER.exists() ? pet.currentStreak : 0;
  const unsigned pages = PET_MANAGER.exists() ? pet.totalPagesRead : 0;
  char summary[96];
  snprintf(summary, sizeof(summary), "%s %u min    %s %u    %s %u", tr(STR_STATS_TODAY), minutes, tr(STR_STATS_STREAK),
           streak, tr(STR_PET_PAGES), pages);
  renderer.drawLine(20, bannerTop, width - 20, bannerTop);
  renderer.drawCenteredText(UI_12_FONT_ID, bannerTop + 14, summary);
  renderer.drawLine(20, bannerTop + 38, width - 20, bannerTop + 38);
  const int top = bannerTop + 48;
  GUI.drawList(renderer, Rect{0, top, width, height - top - metrics.buttonHintsHeight}, kGameCount, selected,
               [names](int i) { return std::string(names[i]); });
  const auto hints = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, hints.btn1, hints.btn2, hints.btn3, hints.btn4);
  renderer.displayBuffer();
}
