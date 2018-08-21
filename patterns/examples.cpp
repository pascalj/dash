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

  // this process
  Process proc;
  std::cout << "BlockPattern<int>" << std::endl;
  std::cout << "\tProcessMap: ";
  name<typename find_node<is_process>::in_tree<Pattern::tree_t, 0, 0>::type>();
  std::cout << "\tbegin(" << proc.index << "): " << p.lbegin(proc) << ", end: " << p.lend(proc) << std::endl;
#ifdef MPI_VERSION
  MPI_Finalize();
#endif
}
