#pragma once

#include <GfxRenderer.h>

#include "pet/PetState.h"

// Renders pet stat bars (hunger, happiness, health, weight, discipline)
// and a status icon row (sleeping, sick, dirty, attention indicators).
// All methods are stateless; pass GfxRenderer and PetState by ref.
class PetStatsPanel {
 public:
  // Draw row of status icons below the pet sprite (x/y = top-left, w = available width)
  static void renderStatusIcons(const GfxRenderer& renderer, const PetState& state, int x, int y, int w);

  // Draw 5 stat bars + care-mistakes counter
  static void renderStats(const GfxRenderer& renderer, const PetState& state, int x, int y, int w);

 private:
  static void drawStatBar(const GfxRenderer& renderer, int x, int y, int barW, const char* label, uint8_t value);
};
