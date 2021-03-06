// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any unsigned
// intellectual property of any third parties.

#ifndef Sapphire_COMPUTE_CUDA_DENSE_GEMMKERNEL_CUH
#define Sapphire_COMPUTE_CUDA_DENSE_GEMMKERNEL_CUH

#include <Sapphire/compute/cudaUtil/CudaParams.cuh>

namespace Sapphire::Compute::Dense::Cuda
{
//! Computes NaiveGemm operation on the chunk (Out = A x B + Out)
//! Chunk represents sub matrix that can be computed using one block
//! Each chunk is composed of tiles which contains 16x16 elements each
//! ByteSize of the chunk is configurable by chunkSize template parameter
//! For example, if chunk size is 4, each block will compute (4x4) x (16x16) =
//! 64x64 chunks each.
//!
//! However, chunk size should be carefully configured considering these
//! constraints
//!     A. size of the shared memory on each SM.
//!         2 x (chunkSize x chunkSize) x (16x16) x sizeof(half) should be
//!         smaller than shared memory size since shared memory is shifted to
//!         prevent bank conflicts
//!     B. Memory alignment on shared memory
//!         Each row in shared memory is aligned to 256 bit (32 bytes).
//!         meaning, 16 x chunkSize x sizeof(half) should be multiple of 32
//!         bytes
//!     Otherwise, static assertion will fail
//!
//! Warp size and block ByteSize should be allocated considering chunk size
//! This kernel requires chunkSize x chunkSize warps in block.x dimension
//! meaning, we need to allocate chunkSize x chunkSize x 32(warp size) threads.
//! (y and z dimension is not used for block)
//! Grid x and y dimension depends on chunks in direction of M and N
//! It should be allocated as (x, y) = (M/chunkSize, N/chunkSize)
//!
//! \param Out : output matrix pointer containing whole matrix
//! \param A : matrix A
//! \param B : matrix B
//! \param paddedK : padded column size of A
//! \param paddedN : padded column size of B
//! \param chunkIdxK : index of K
//! \param chunkSize : size of the chunk
__global__ void WmmaGemm(float* Out, const float* A, const float* B,
                         const float* C, unsigned int paddedK,
                         unsigned int paddedN, unsigned int chunkIdxK,
                         const unsigned int chunkSize);

//! Required thread dimension is (tileDim x chunkSize) x (tileDim x chunkSize)
//! each thread computes each output entry in the chunk
//! Required block size is chunkDimM x chunkDimN
__global__ void Gemm(float* out, const float* A, const float* B, const float* C,
                     unsigned int paddedM, unsigned int paddedN,
                     unsigned int paddedK, unsigned int chunkIdxK,
                     const unsigned int chunkSize,
                     const unsigned int chunkDimN);

__global__ void GemmSimple(float* out, const float* A, const float* B, const float* C,
                           unsigned int paddedM, unsigned int paddedN,
                           unsigned int paddedK);
}  // namespace Sapphire::Compute::Cuda::Dense
#endif
