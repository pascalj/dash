#ifndef PATTERNS__TESTS__TEST_ENTITY_H
#define PATTERNS__TESTS__TEST_ENTITY_H

#include <cstddef>

struct TestEntity {
  std::size_t _index = 0;
  std::size_t _total = 1;

  auto index() {
    return _index;
  }

  auto total() {
    return _total;
  }
};

#endif
