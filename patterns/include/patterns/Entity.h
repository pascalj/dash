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

struct Entity {
  Entity() = default;

  Entity(size_t index, size_t total)
    : index(index)
    , total(total)
  {
  }

  // Index of the current entity
  size_t index;
  // Number of total entities of this type
  size_t total;
};

// Number of GPUs
struct GPU : Entity {
  GPU(size_t index, size_t total) : Entity(index, total) {}
};
struct Process : Entity {
  Process() : Entity(0, 1) {
#ifdef MPI_VERSION
    MPI_Comm_rank(MPI_COMM_WORLD, &index);
    MPI_Comm_size(MPI_COMM_WORLD, &total);
#endif
  }

  Process(size_t index, size_t total) : Entity(index, total) {}
};
struct None : Entity {
  int index = 0;
  int total = 0;
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
