#include "GameScores.h"

#include <HalStorage.h>
#include <Logging.h>
#include <Serialization.h>

namespace {
constexpr uint8_t SCORES_FILE_VERSION = 1;
constexpr char SCORES_FILE[] = "/.crosspoint/game_scores.bin";
}  // namespace

GameScores GameScores::instance;

bool GameScores::saveToFile() const {
  Storage.mkdir("/.crosspoint");
  HalFile file;
  if (!Storage.openFileForWrite("GSC", SCORES_FILE, file)) {
    LOG_ERR("GSC", "Failed to open game_scores.bin for write");
    return false;
  }
  const uint8_t version = SCORES_FILE_VERSION;
  serialization::writePod(file, version);
  serialization::writePod(file, snakeHigh);
  serialization::writePod(file, best2048);
  serialization::writePod(file, mazeBest[0]);
  serialization::writePod(file, mazeBest[1]);
  serialization::writePod(file, mazeBest[2]);
  return true;
}

bool GameScores::loadFromFile() {
  HalFile file;
  if (!Storage.openFileForRead("GSC", SCORES_FILE, file)) {
    return false;  // first boot — file doesn't exist yet
  }
  uint8_t version = 0;
  if (!serialization::readPod(file, version) || version != SCORES_FILE_VERSION) {
    LOG_ERR("GSC", "Unknown game_scores.bin version %u", version);
    return false;
  }
  uint32_t savedSnake = 0;
  uint32_t saved2048 = 0;
  uint16_t savedMaze[3] = {};
  if (!serialization::readPod(file, savedSnake) || !serialization::readPod(file, saved2048) ||
      !serialization::readPod(file, savedMaze[0]) || !serialization::readPod(file, savedMaze[1]) ||
      !serialization::readPod(file, savedMaze[2])) {
    LOG_ERR("GSC", "Truncated game scores; ignoring file");
    return false;
  }
  snakeHigh = savedSnake;
  best2048 = saved2048;
  for (int i = 0; i < 3; i++) mazeBest[i] = savedMaze[i];
  return true;
}
