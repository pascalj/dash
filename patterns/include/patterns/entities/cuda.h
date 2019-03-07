#ifndef ENTITIES__CUDA_H
#define ENTITIES__CUDA_H

#include <cstddef>
#include <cuda_runtime.h>

namespace patterns {

class CudaEntity {
  size_t index() const
  {
    int device;
    cudaGetDevice(&device);
    return device;
  }

  size_t total() const
  {
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    return deviceCount;
  }
};

}

#endif
