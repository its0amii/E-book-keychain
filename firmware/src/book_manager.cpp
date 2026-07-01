// book_manager.cpp
// ============================================================
#include "book_manager.h"

BookManager bookManager;

void BookManager::refresh() {
  m_books.clear();
  storage.listBooks(m_books);
}

bool BookManager::indexOfPath(const String& path, uint8_t& out) const {
  for (uint8_t i = 0; i < m_books.size(); i++) {
    if (m_books[i].path == path) { out = i; return true; }
  }
  return false;
}
