// book_manager.h — discover books on the SD card
// ============================================================
#pragma once
#include <Arduino.h>
#include <vector>
#include "types.h"
#include "storage.h"

class BookManager {
public:
  // Re-scans /books and updates the internal list.
  void refresh();

  // Accessors
  const std::vector<BookEntry>& books() const { return m_books; }
  uint8_t count() const { return m_books.size(); }
  const BookEntry& at(uint8_t i) const { return m_books[i]; }

  // Convenience helpers
  bool indexOfPath(const String& path, uint8_t& out) const;

private:
  std::vector<BookEntry> m_books;
};

extern BookManager bookManager;
