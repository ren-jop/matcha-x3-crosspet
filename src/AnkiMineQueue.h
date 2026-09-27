#pragma once

// SD-backed mine outbox. The desktop companion imports into the user's existing
// Anki model and syncs it to AnkiWeb for AnkiMobile.
class AnkiMineQueue {
 public:
  static bool enqueue(const char* expression, const char* reading, const char* meaning);
  static void pump();
};
