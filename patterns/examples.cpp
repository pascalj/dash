#ifdef USE_MPI
#include "mpi.h"
#endif

#include "patterns/EntityPattern.h"
#include "patterns/Entity.h"
#include <iostream>


using ValueType = int;

template<typename T>
void name() {
  std::cout << __PRETTY_FUNCTION__ << std::endl;
}

int main() {
#ifdef MPI_VERSION
  MPI_Init(NULL, NULL);
#endif

  using Pattern = EntityPattern<
    ProxyPattern<3, Process, int>
    >;

  Pattern p;

#ifdef MPI_VERSION
  MPI_Finalize();
#endif
}
