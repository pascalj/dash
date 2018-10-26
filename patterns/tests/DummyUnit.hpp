#include <cstddef>

struct DummyUnit {
  size_t total = 3;
  size_t index = 0;

  static const DummyUnit current() {
    return DummyUnit();
  }
};
