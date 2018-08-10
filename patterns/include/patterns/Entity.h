#ifndef PATTERNS__ENTITY_H
#define PATTERNS__ENTITY_H


/**
 * An EntityClass is used to describe what kind of computational unit
 * may be assigned to a given node in the pattern tree.
 *
 * Examples of EntityClasses: Processes, Threads, GPUs, FPGAs.
 *
 * A concrete instance of an EntityClass is an Entity, e.g. one specific
 * process or GPU.
 */
template<typename Entity>
struct EntityClass {
};


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
};
struct None : Entity<None> {
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





using GPUEntityClass = EntityClass<GPU>;
using ProcessEntityClass = EntityClass<Process>;

#endif
