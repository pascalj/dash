#ifndef ENTITIES__CUDA_H
#define ENTITIES__CUDA_H

#include <cstddef>

namespace patterns {

struct CudaEntity {
  FN_HOST_ACC size_t index() const
  {
    int device;
    cudaGetDevice(&device);
    return device;
  }

  FN_HOST_ACC size_t total() const
  {
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    return deviceCount;
  }
};

}

#endif
