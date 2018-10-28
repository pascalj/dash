#ifdef USE_MPI
#include "mpi.h"
#endif

#include "patterns/BlockPattern.h"
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
  using Pattern = BlockPattern<ValueType>;
  Pattern p(100);

  std::cout << "lbegin: " << p.lbegin(Process()) << std::endl;
  std::cout << "lend: " << p.lend(Process()) << std::endl;

#ifdef MPI_VERSION
  MPI_Finalize();
#endif
}
