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
  using Tree = BalancedNode<
      EmptyEntity,
      BalancedNode<GPU, LeafNode<Process>, LeafNode<Process>>,
      BalancedNode<GPU, LeafNode<Process>, LeafNode<Process>>>;
  BlockPattern<Process, Tree> p(100);

  const auto nprocs = 4;
  const auto ngpu = 2;

  for (int i = 0; i < nprocs; i++) {
    std::cout << "Process[" << i << "]: [" << p.lbegin(Process(i, nprocs)) << ", "
              << p.lend(Process(i, nprocs)) << ")" << std::endl;
  }
  for (int i = 0; i < ngpu; i++) {
    std::cout << "GPU[" << i << "]: [" << p.lbegin(GPU(i, ngpu)) << ", "
              << p.lend(GPU(i, ngpu)) << ")" << std::endl;
  }

#ifdef MPI_VERSION
  MPI_Finalize();
#endif
}
