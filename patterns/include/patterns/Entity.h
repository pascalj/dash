#ifndef PATTERNS__ENTITY_H
#define PATTERNS__ENTITY_H


#include <iostream>

#ifdef USE_MPI
#include "mpi.h"
#endif


/**
 * An EntityClass is used to describe what kind of computational unit
 * may be assigned to a given node in the pattern tree.
 *
 * Examples of EntityClasses: Processes, Threads, GPUs, FPGAs.
 *
 * A concrete instance of an EntityClass is an Entity, e.g. one specific
 * process or GPU.
 */


struct EmptyEntity {};

template<typename EntityType>
struct Entity {
  // Index of the current entity
  int index;
  // Number of total entities of this type
  int total;
};

// Number of GPUs
struct GPU : Entity<GPU> {
};
struct Process : Entity<Process> {
#ifdef MPI_VERSION
  Process() {
    index = 1;
    total = 4;
    /* MPI_Comm_rank(MPI_COMM_WORLD, &index); */
    /* MPI_Comm_size(MPI_COMM_WORLD, &total); */
  }
#endif
};
struct None : Entity<None> {
  int index = 0;
  int total = 1;
};


template<typename Entity>
struct entity_name {
  static constexpr const char* value = "Unknown";
};

template<>
struct entity_name<None> {
  static constexpr const char* value = "None";
};

template<>
struct entity_name<GPU> {
  static constexpr const char* value = "GPU";
};

template<>
struct entity_name<Process> {
  static constexpr const char* value = "Process";
};


#endif
