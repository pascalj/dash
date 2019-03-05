#ifndef ENTITIES__DUMMY_H
#define ENTITIES__DUMMY_H


#include <cstddef>

namespace patterns {

class DummyEntity {
  size_t _total   = 5;
  size_t _current = 2;

  size_t current() const
  {
    return _current;
  }

  size_t total() const
  {
    return _total;
  }
};

}

#endif
