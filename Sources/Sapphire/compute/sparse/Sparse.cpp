// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Sapphire/compute/cudaUtil/Memory.hpp>
#include <Sapphire/compute/sparse/Sparse.hpp>
#include <Sapphire/compute/sparse/SparseMatrix.hpp>
#include <Sapphire/util/MemoryManager.hpp>

namespace Sapphire::Compute
{
using namespace Util;

void DeepAllocateSparseHost(SparseMatrix** sparseMatrixArray, const uint32_t m,
                            const uint32_t n, const uint32_t nnz[],
                            uint32_t numMatrices)
{
    *sparseMatrixArray = static_cast<SparseMatrix*>(
        MemoryManager::GetMemoryHost(sizeof(SparseMatrix) * numMatrices));
    SparseMatrix* targetArray = *sparseMatrixArray;

    for (uint32_t i = 0; i < numMatrices; ++i)
    {
        targetArray[i].M = m;
        targetArray[i].N = n;
        targetArray[i].NNZ = nnz[i];
        targetArray[i].V = static_cast<float*>(
            MemoryManager::GetMemoryHost(nnz[i] * sizeof(float)));
        targetArray[i].COL = static_cast<uint32_t*>(
            MemoryManager::GetMemoryHost(nnz[i] * sizeof(uint32_t)));
        targetArray[i].ROW = static_cast<uint32_t*>(
            MemoryManager::GetMemoryHost((m + 1) * sizeof(uint32_t)));
    }
}

void DeepAllocateLoadDistHost(LoadDistMatrix** loadDistArray,
                              SparseMatrix* sparseArray, uint32_t numMatrices)
{
    *loadDistArray = static_cast<LoadDistMatrix*>(
        MemoryManager::GetMemoryHost(sizeof(LoadDistMatrix) * numMatrices));
    LoadDistMatrix* targetArray = *loadDistArray;

    for (uint32_t i = 0; i < numMatrices; ++i)
    {
        targetArray[i].M = sparseArray[i].M;
        targetArray[i].N = sparseArray[i].N;
        targetArray[i].NNZ = sparseArray[i].NNZ;
        targetArray[i].Load =
            static_cast<uint32_t*>(MemoryManager::GetMemoryHost(
                sparseArray[i].NNZ * sizeof(uint32_t)));
        targetArray[i].COL =
            static_cast<uint32_t*>(MemoryManager::GetMemoryHost(
                sparseArray[i].NNZ * sizeof(uint32_t)));
        targetArray[i].ROW =
            static_cast<uint32_t*>(MemoryManager::GetMemoryHost(
                (sparseArray[i].M + 1) * sizeof(uint32_t)));
    }
}

void DeepFreeSparseHost(SparseMatrix* sparseMatrixArray, uint32_t numMatrices)
{
    for (uint32_t i = 0; i < numMatrices; ++i)
    {
        MemoryManager::DeReferenceHost(
            static_cast<void*>(sparseMatrixArray[i].V));
        MemoryManager::DeReferenceHost(
            static_cast<void*>(sparseMatrixArray[i].COL));
        MemoryManager::DeReferenceHost(
            static_cast<void*>(sparseMatrixArray[i].ROW));
    }

    MemoryManager::DeReferenceHost(sparseMatrixArray);
}

void DeepFreeLoadDistHost(LoadDistMatrix* loadDistArray, uint32_t numMatrices)
{
    for (uint32_t i = 0; i < numMatrices; ++i)
    {
        MemoryManager::DeReferenceHost(
            static_cast<void*>(loadDistArray[i].Load));
        MemoryManager::DeReferenceHost(
            static_cast<void*>(loadDistArray[i].COL));
        MemoryManager::DeReferenceHost(
            static_cast<void*>(loadDistArray[i].ROW));
    }

    MemoryManager::DeReferenceHost(loadDistArray);
}

void DeepAllocateSparseCuda(SparseMatrix** deviceSparseArray,
                            const SparseMatrix* hostSparseMatrixArray,
                            uint32_t numMatrices, int deviceId)
{
    *deviceSparseArray =
        static_cast<SparseMatrix*>(MemoryManager::GetMemoryCuda(
            sizeof(SparseMatrix) * numMatrices, deviceId));
    auto* hostDstBuffer = static_cast<SparseMatrix*>(
        MemoryManager::GetMemoryHost(sizeof(SparseMatrix) * numMatrices));

    for (uint32_t i = 0; i < numMatrices; ++i)
    {
        const auto nnz = hostSparseMatrixArray[i].NNZ;
        hostDstBuffer[i].M = hostSparseMatrixArray[i].M;
        hostDstBuffer[i].N = hostSparseMatrixArray[i].N;
        hostDstBuffer[i].NNZ = hostSparseMatrixArray[i].NNZ;
        hostDstBuffer[i].V = static_cast<float*>(MemoryManager::GetMemoryCuda(
            (!nnz ? 1 : nnz) * sizeof(float), deviceId));
        hostDstBuffer[i].COL =
            static_cast<uint32_t*>(MemoryManager::GetMemoryCuda(
                (!nnz ? 1 : nnz) * sizeof(uint32_t), deviceId));
        hostDstBuffer[i].ROW =
            static_cast<uint32_t*>(MemoryManager::GetMemoryCuda(
                (hostSparseMatrixArray[i].M + 1) * sizeof(uint32_t), deviceId));
    }

    Cuda::CopyHostToDevice(*deviceSparseArray, hostDstBuffer,
                           sizeof(SparseMatrix) * numMatrices);
    MemoryManager::DeReferenceHost(hostDstBuffer);
}

void DeepAllocateLoadDistCuda(LoadDistMatrix** deviceLoadDistArray,
                              LoadDistMatrix* hostLoadDistArray,
                              uint32_t numMatrices, int deviceId)
{
    *deviceLoadDistArray =
        static_cast<LoadDistMatrix*>(MemoryManager::GetMemoryCuda(
            sizeof(LoadDistMatrix) * numMatrices, deviceId));
    auto* hostDstBuffer = static_cast<LoadDistMatrix*>(
        MemoryManager::GetMemoryHost(sizeof(LoadDistMatrix) * numMatrices));

    for (uint32_t i = 0; i < numMatrices; ++i)
    {
        hostDstBuffer[i].M = hostLoadDistArray[i].M;
        hostDstBuffer[i].N = hostLoadDistArray[i].N;
        hostDstBuffer[i].NNZ = hostLoadDistArray[i].NNZ;
        hostDstBuffer[i].Load =
            static_cast<uint32_t*>(MemoryManager::GetMemoryCuda(
                hostLoadDistArray[i].NNZ * sizeof(uint32_t), deviceId));
        hostDstBuffer[i].COL =
            static_cast<uint32_t*>(MemoryManager::GetMemoryCuda(
                hostLoadDistArray[i].NNZ * sizeof(uint32_t), deviceId));
        hostDstBuffer[i].ROW =
            static_cast<uint32_t*>(MemoryManager::GetMemoryCuda(
                (hostLoadDistArray[i].M + 1) * sizeof(uint32_t), deviceId));
    }

    Cuda::CopyHostToDevice(*deviceLoadDistArray, hostDstBuffer,
                           sizeof(LoadDistMatrix) * numMatrices);
    MemoryManager::DeReferenceHost(hostDstBuffer);
}

void DeepFreeSparseCuda(SparseMatrix* deviceSparseArray, uint32_t numMatrices,
                        int deviceId)
{
    auto* hostBuffer = static_cast<SparseMatrix*>(
        MemoryManager::GetMemoryHost(sizeof(SparseMatrix) * numMatrices));
    Cuda::CopyDeviceToHost(hostBuffer, deviceSparseArray,
                           sizeof(SparseMatrix) * numMatrices);
    for (uint32_t i = 0; i < numMatrices; ++i)
    {
        MemoryManager::DeReferenceCuda(hostBuffer[i].V, deviceId);
        MemoryManager::DeReferenceCuda(hostBuffer[i].COL, deviceId);
        MemoryManager::DeReferenceCuda(hostBuffer[i].ROW, deviceId);
    }
    MemoryManager::DeReferenceHost(hostBuffer);
    MemoryManager::DeReferenceCuda(deviceSparseArray, deviceId);
}

void DeepFreeLoadDistCuda(LoadDistMatrix* deviceLoadDistArray,
                          uint32_t numMatrices, int deviceId)
{
    auto* hostBuffer = static_cast<LoadDistMatrix*>(
        MemoryManager::GetMemoryHost(sizeof(LoadDistMatrix) * numMatrices));
    Cuda::CopyDeviceToHost(hostBuffer, deviceLoadDistArray,
                           sizeof(LoadDistMatrix) * numMatrices);
    for (uint32_t i = 0; i < numMatrices; ++i)
    {
        MemoryManager::DeReferenceCuda(hostBuffer[i].Load, deviceId);
        MemoryManager::DeReferenceCuda(hostBuffer[i].COL, deviceId);
        MemoryManager::DeReferenceCuda(hostBuffer[i].ROW, deviceId);
    }
    MemoryManager::DeReferenceHost(hostBuffer);
    MemoryManager::DeReferenceCuda(deviceLoadDistArray, deviceId);
}

void DeepCopyDeviceToDevice(SparseMatrix* deviceDstArray,
                            SparseMatrix* deviceSrcArray, uint32_t numMatrices,
                            int deviceId)
{
    auto* dstArrayBuffer = static_cast<SparseMatrix*>(
        MemoryManager::GetMemoryHost(numMatrices * sizeof(SparseMatrix)));
    auto* srcArrayBuffer = static_cast<SparseMatrix*>(
        MemoryManager::GetMemoryHost(numMatrices * sizeof(SparseMatrix)));

    Cuda::CudaSetDevice(deviceId);
    Cuda::CopyDeviceToHost(srcArrayBuffer, deviceSrcArray,
                           numMatrices * sizeof(SparseMatrix));
    Cuda::CopyDeviceToHost(dstArrayBuffer, deviceDstArray,
                           numMatrices * sizeof(SparseMatrix));

    for (uint32_t idx = 0; idx < numMatrices; ++idx)
    {
        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].COL, deviceId);
        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].V, deviceId);
        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].ROW, deviceId);

        dstArrayBuffer[idx].COL =
            static_cast<uint32_t*>(MemoryManager::GetMemoryCuda(
                srcArrayBuffer[idx].NNZ * sizeof(uint32_t), deviceId));
        dstArrayBuffer[idx].V =
            static_cast<float*>(MemoryManager::GetMemoryCuda(
                srcArrayBuffer[idx].NNZ * sizeof(float), deviceId));
        dstArrayBuffer[idx].ROW =
            static_cast<uint32_t*>(MemoryManager::GetMemoryCuda(
                (srcArrayBuffer[idx].M + 1) * sizeof(uint32_t), deviceId));

        dstArrayBuffer[idx].M = srcArrayBuffer[idx].M;
        dstArrayBuffer[idx].N = srcArrayBuffer[idx].N;
        dstArrayBuffer[idx].NNZ = srcArrayBuffer[idx].NNZ;

        Cuda::CopyDeviceToDevice(dstArrayBuffer[idx].COL,
                                 srcArrayBuffer[idx].COL,
                                 srcArrayBuffer[idx].NNZ * sizeof(uint32_t));
        Cuda::CopyDeviceToDevice(dstArrayBuffer[idx].V, srcArrayBuffer[idx].V,
                                 srcArrayBuffer[idx].NNZ * sizeof(float));
        Cuda::CopyDeviceToDevice(
            dstArrayBuffer[idx].ROW, srcArrayBuffer[idx].ROW,
            (srcArrayBuffer[idx].M + 1) * sizeof(uint32_t));
    }

    Cuda::CopyHostToDevice(deviceDstArray, dstArrayBuffer,
                           numMatrices * sizeof(SparseMatrix));

    MemoryManager::DeReferenceHost(srcArrayBuffer);
    MemoryManager::DeReferenceHost(dstArrayBuffer);
}

void DeepCopyDeviceToDevice(LoadDistMatrix* deviceDstArray,
                            LoadDistMatrix* deviceSrcArray,
                            uint32_t numMatrices, int deviceId)
{
    auto* dstArrayBuffer = static_cast<LoadDistMatrix*>(
        MemoryManager::GetMemoryHost(numMatrices * sizeof(LoadDistMatrix)));
    auto* srcArrayBuffer = static_cast<LoadDistMatrix*>(
        MemoryManager::GetMemoryHost(numMatrices * sizeof(LoadDistMatrix)));

    Cuda::CudaSetDevice(deviceId);
    Cuda::CopyDeviceToHost(dstArrayBuffer, deviceDstArray,
                           numMatrices * sizeof(LoadDistMatrix));
    Cuda::CopyDeviceToHost(srcArrayBuffer, deviceSrcArray,
                           numMatrices * sizeof(LoadDistMatrix));

    for (uint32_t idx = 0; idx < numMatrices; ++idx)
    {
        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].COL, deviceId);
        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].Load, deviceId);
        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].ROW, deviceId);

        dstArrayBuffer[idx].COL =
            static_cast<uint32_t*>(MemoryManager::GetMemoryCuda(
                srcArrayBuffer[idx].NNZ * sizeof(uint32_t), deviceId));
        dstArrayBuffer[idx].Load =
            static_cast<uint32_t*>(MemoryManager::GetMemoryCuda(
                srcArrayBuffer[idx].NNZ * sizeof(uint32_t), deviceId));
        dstArrayBuffer[idx].ROW =
            static_cast<uint32_t*>(MemoryManager::GetMemoryCuda(
                (srcArrayBuffer[idx].M + 1) * sizeof(uint32_t), deviceId));

        dstArrayBuffer[idx].M = srcArrayBuffer[idx].M;
        dstArrayBuffer[idx].N = srcArrayBuffer[idx].N;
        dstArrayBuffer[idx].NNZ = srcArrayBuffer[idx].NNZ;

        Cuda::CopyDeviceToDevice(dstArrayBuffer[idx].COL,
                                 srcArrayBuffer[idx].COL,
                                 srcArrayBuffer[idx].NNZ * sizeof(uint32_t));
        Cuda::CopyDeviceToDevice(dstArrayBuffer[idx].Load,
                                 srcArrayBuffer[idx].Load,
                                 srcArrayBuffer[idx].NNZ * sizeof(uint32_t));
        Cuda::CopyDeviceToDevice(
            dstArrayBuffer[idx].ROW, srcArrayBuffer[idx].ROW,
            (srcArrayBuffer[idx].M + 1) * sizeof(uint32_t));
    }

    Cuda::CopyHostToDevice(deviceDstArray, dstArrayBuffer,
                           numMatrices * sizeof(LoadDistMatrix));

    MemoryManager::DeReferenceHost(srcArrayBuffer);
    MemoryManager::DeReferenceHost(dstArrayBuffer);
}

void DeepCopyHostToDevice(SparseMatrix* deviceDstArray,
                          const SparseMatrix* hostSrcArray,
                          uint32_t numMatrices, int deviceId)
{
    auto* dstArrayBuffer = static_cast<SparseMatrix*>(
        MemoryManager::GetMemoryHost(numMatrices * sizeof(SparseMatrix)));

    Cuda::CudaSetDevice(deviceId);
    Cuda::CopyDeviceToHost(dstArrayBuffer, deviceDstArray,
                           numMatrices * sizeof(SparseMatrix));

    for (uint32_t idx = 0; idx < numMatrices; ++idx)
    {
        const auto m = hostSrcArray[idx].M;
        const auto n = hostSrcArray[idx].N;
        const auto nnz = hostSrcArray[idx].NNZ;

        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].V, deviceId);
        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].COL, deviceId);
        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].ROW, deviceId);

        dstArrayBuffer[idx].V = static_cast<float*>(
            MemoryManager::GetMemoryCuda(sizeof(float) * nnz, deviceId));
        dstArrayBuffer[idx].COL = static_cast<uint32_t*>(
            MemoryManager::GetMemoryCuda(sizeof(uint32_t) * nnz, deviceId));
        dstArrayBuffer[idx].ROW = static_cast<uint32_t*>(
            MemoryManager::GetMemoryCuda(sizeof(uint32_t) * (m + 1), deviceId));

        dstArrayBuffer[idx].M = m;
        dstArrayBuffer[idx].N = n;
        dstArrayBuffer[idx].NNZ = nnz;

        Cuda::CopyHostToDevice(dstArrayBuffer[idx].V, hostSrcArray[idx].V,
                               sizeof(float) * nnz);
        Cuda::CopyHostToDevice(dstArrayBuffer[idx].COL, hostSrcArray[idx].COL,
                               sizeof(uint32_t) * nnz);
        Cuda::CopyHostToDevice(dstArrayBuffer[idx].ROW, hostSrcArray[idx].ROW,
                               sizeof(uint32_t) * (m + 1));
    }
    Cuda::CopyHostToDevice(deviceDstArray, dstArrayBuffer,
                           sizeof(SparseMatrix) * numMatrices);
    MemoryManager::DeReferenceHost(dstArrayBuffer);
}

void DeepCopyHostToDevice(LoadDistMatrix* deviceDstArray,
                          LoadDistMatrix* hostSrcArray, uint32_t numMatrices,
                          int deviceId)
{
    auto* dstArrayBuffer = static_cast<LoadDistMatrix*>(
        MemoryManager::GetMemoryHost(numMatrices * sizeof(LoadDistMatrix)));

    Cuda::CudaSetDevice(deviceId);
    Cuda::CopyDeviceToHost(dstArrayBuffer, deviceDstArray,
                           numMatrices * sizeof(LoadDistMatrix));

    for (uint32_t idx = 0; idx < numMatrices; ++idx)
    {
        const auto m = hostSrcArray[idx].M;
        const auto n = hostSrcArray[idx].N;
        const auto nnz = hostSrcArray[idx].NNZ;

        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].Load, deviceId);
        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].COL, deviceId);
        dstArrayBuffer[idx].Load = static_cast<uint32_t*>(
            MemoryManager::GetMemoryCuda(sizeof(uint32_t) * nnz, deviceId));
        dstArrayBuffer[idx].COL = static_cast<uint32_t*>(
            MemoryManager::GetMemoryCuda(sizeof(uint32_t) * nnz, deviceId));

        MemoryManager::DeReferenceCuda(dstArrayBuffer[idx].ROW, deviceId);
        dstArrayBuffer[idx].ROW = static_cast<uint32_t*>(
            MemoryManager::GetMemoryCuda(sizeof(uint32_t) * (m + 1), deviceId));

        dstArrayBuffer[idx].M = m;
        dstArrayBuffer[idx].N = n;
        dstArrayBuffer[idx].NNZ = nnz;

        Cuda::CopyHostToDevice(dstArrayBuffer[idx].Load, hostSrcArray[idx].Load,
                               sizeof(uint32_t) * nnz);
        Cuda::CopyHostToDevice(dstArrayBuffer[idx].COL, hostSrcArray[idx].COL,
                               sizeof(uint32_t) * nnz);
        Cuda::CopyHostToDevice(dstArrayBuffer[idx].ROW, hostSrcArray[idx].ROW,
                               sizeof(uint32_t) * (m + 1));
    }
    Cuda::CopyHostToDevice(deviceDstArray, dstArrayBuffer,
                           sizeof(LoadDistMatrix) * numMatrices);
    MemoryManager::DeReferenceHost(dstArrayBuffer);
}

void DeepCopyDeviceToHost(SparseMatrix* hostDstArray,
                          SparseMatrix* deviceSrcArray, uint32_t numMatrices,
                          int deviceId)
{
    auto* srcArrayBuffer = static_cast<SparseMatrix*>(
        MemoryManager::GetMemoryHost(numMatrices * sizeof(SparseMatrix)));

    Cuda::CudaSetDevice(deviceId);
    Cuda::CopyDeviceToHost(srcArrayBuffer, deviceSrcArray,
                           numMatrices * sizeof(SparseMatrix));

    for (uint32_t idx = 0; idx < numMatrices; ++idx)
    {
        MemoryManager::DeReferenceHost(hostDstArray[idx].COL);
        MemoryManager::DeReferenceHost(hostDstArray[idx].V);
        hostDstArray[idx].COL =
            static_cast<uint32_t*>(MemoryManager::GetMemoryHost(
                srcArrayBuffer[idx].NNZ * sizeof(uint32_t)));
        hostDstArray[idx].V = static_cast<float*>(MemoryManager::GetMemoryHost(
            srcArrayBuffer[idx].NNZ * sizeof(float)));

        MemoryManager::DeReferenceHost(hostDstArray[idx].ROW);
        hostDstArray[idx].ROW =
            static_cast<uint32_t*>(MemoryManager::GetMemoryHost(
                (srcArrayBuffer[idx].M + 1) * sizeof(uint32_t)));

        hostDstArray[idx].M = srcArrayBuffer[idx].M;
        hostDstArray[idx].N = srcArrayBuffer[idx].N;
        hostDstArray[idx].NNZ = srcArrayBuffer[idx].NNZ;

        Cuda::CopyDeviceToHost(hostDstArray[idx].COL, srcArrayBuffer[idx].COL,
                               srcArrayBuffer[idx].NNZ * sizeof(uint32_t));
        Cuda::CopyDeviceToHost(hostDstArray[idx].V, srcArrayBuffer[idx].V,
                               srcArrayBuffer[idx].NNZ * sizeof(float));
        Cuda::CopyDeviceToHost(hostDstArray[idx].ROW, srcArrayBuffer[idx].ROW,
                               (srcArrayBuffer[idx].M + 1) * sizeof(uint32_t));
    }

    MemoryManager::DeReferenceHost(srcArrayBuffer);
}

void DeepCopyDeviceToHost(LoadDistMatrix* hostDstArray,
                          LoadDistMatrix* deviceSrcArray, uint32_t numMatrices,
                          int deviceId)
{
    auto* srcArrayBuffer = static_cast<LoadDistMatrix*>(
        MemoryManager::GetMemoryHost(numMatrices * sizeof(LoadDistMatrix)));

    Cuda::CudaSetDevice(deviceId);
    Cuda::CopyDeviceToHost(srcArrayBuffer, deviceSrcArray,
                           numMatrices * sizeof(LoadDistMatrix));

    for (uint32_t idx = 0; idx < numMatrices; ++idx)
    {
        MemoryManager::DeReferenceHost(hostDstArray[idx].COL);
        MemoryManager::DeReferenceHost(hostDstArray[idx].Load);
        hostDstArray[idx].COL =
            static_cast<uint32_t*>(MemoryManager::GetMemoryHost(
                srcArrayBuffer[idx].NNZ * sizeof(uint32_t)));
        hostDstArray[idx].Load =
            static_cast<uint32_t*>(MemoryManager::GetMemoryHost(
                srcArrayBuffer[idx].NNZ * sizeof(uint32_t)));

        MemoryManager::DeReferenceHost(hostDstArray[idx].ROW);
        hostDstArray[idx].ROW =
            static_cast<uint32_t*>(MemoryManager::GetMemoryHost(
                (srcArrayBuffer[idx].M + 1) * sizeof(uint32_t)));

        hostDstArray[idx].M = srcArrayBuffer[idx].M;
        hostDstArray[idx].N = srcArrayBuffer[idx].N;
        hostDstArray[idx].NNZ = srcArrayBuffer[idx].NNZ;

        Cuda::CopyDeviceToHost(hostDstArray[idx].COL, srcArrayBuffer[idx].COL,
                               srcArrayBuffer[idx].NNZ * sizeof(uint32_t));
        Cuda::CopyDeviceToHost(hostDstArray[idx].Load, srcArrayBuffer[idx].Load,
                               srcArrayBuffer[idx].NNZ * sizeof(uint32_t));
        Cuda::CopyDeviceToHost(hostDstArray[idx].ROW, srcArrayBuffer[idx].ROW,
                               (srcArrayBuffer[idx].M + 1) * sizeof(uint32_t));
    }

    MemoryManager::DeReferenceHost(srcArrayBuffer);
}

void DeepCopyHostToHost(SparseMatrix* hostDstArray, SparseMatrix* hostSrcArray,
                        uint32_t numMatrices)
{
    for (uint32_t matrixIdx = 0; matrixIdx < numMatrices; ++matrixIdx)
    {
        const auto nnz = hostSrcArray[matrixIdx].NNZ;
        const auto m = hostSrcArray[matrixIdx].M;
        const auto n = hostSrcArray[matrixIdx].N;

        MemoryManager::DeReferenceHost(hostDstArray[matrixIdx].V);
        MemoryManager::DeReferenceHost(hostDstArray[matrixIdx].COL);

        hostDstArray[matrixIdx].V = static_cast<float*>(
            MemoryManager::GetMemoryHost(nnz * sizeof(float)));
        hostDstArray[matrixIdx].COL = static_cast<uint32_t*>(
            MemoryManager::GetMemoryHost(nnz * sizeof(uint32_t)));

        MemoryManager::DeReferenceHost(hostDstArray[matrixIdx].ROW);
        hostDstArray[matrixIdx].ROW = static_cast<uint32_t*>(
            MemoryManager::GetMemoryHost((m + 1) * sizeof(uint32_t)));

        for (uint32_t i = 0; i < nnz; ++i)
        {
            hostDstArray[matrixIdx].COL[i] = hostSrcArray[matrixIdx].COL[i];
            hostDstArray[matrixIdx].V[i] = hostSrcArray[matrixIdx].V[i];
        }

        for (uint32_t i = 0; i < m + 1; ++i)
        {
            hostDstArray[matrixIdx].ROW[i] = hostSrcArray[matrixIdx].ROW[i];
        }

        hostDstArray[matrixIdx].NNZ = nnz;
        hostDstArray[matrixIdx].M = m;
        hostDstArray[matrixIdx].N = n;
    }
}

void DeepCopyHostToHost(LoadDistMatrix* hostDstArray,
                        LoadDistMatrix* hostSrcArray, uint32_t numMatrices)
{
    for (uint32_t matrixIdx = 0; matrixIdx < numMatrices; ++matrixIdx)
    {
        const auto nnz = hostSrcArray[matrixIdx].NNZ;
        const auto m = hostSrcArray[matrixIdx].M;
        const auto n = hostSrcArray[matrixIdx].N;

        MemoryManager::DeReferenceHost(hostDstArray[matrixIdx].Load);
        MemoryManager::DeReferenceHost(hostDstArray[matrixIdx].COL);

        hostDstArray[matrixIdx].Load = static_cast<uint32_t*>(
            MemoryManager::GetMemoryHost(nnz * sizeof(uint32_t)));
        hostDstArray[matrixIdx].COL = static_cast<uint32_t*>(
            MemoryManager::GetMemoryHost(nnz * sizeof(uint32_t)));

        MemoryManager::DeReferenceHost(hostDstArray[matrixIdx].ROW);
        hostDstArray[matrixIdx].ROW = static_cast<uint32_t*>(
            MemoryManager::GetMemoryHost((m + 1) * sizeof(uint32_t)));

        for (uint32_t i = 0; i < nnz; ++i)
        {
            hostDstArray[matrixIdx].COL[i] = hostSrcArray[matrixIdx].COL[i];
            hostDstArray[matrixIdx].Load[i] = hostSrcArray[matrixIdx].Load[i];
        }

        for (uint32_t i = 0; i < m + 1; ++i)
        {
            hostDstArray[matrixIdx].ROW[i] = hostSrcArray[matrixIdx].ROW[i];
        }

        hostDstArray[matrixIdx].NNZ = nnz;
        hostDstArray[matrixIdx].M = m;
        hostDstArray[matrixIdx].N = n;
    }
}

void CreateSparseMatrixWithDenseMatrix(SparseMatrix** dst, const float* src,
                                       uint32_t m, uint32_t n, uint32_t paddedN,
                                       uint32_t numMatrices)
{
    *dst = static_cast<SparseMatrix*>(
        MemoryManager::GetMemoryHost(sizeof(SparseMatrix) * numMatrices));
    auto* dstPtr = *dst;

#pragma omp parallel for default(none) \
    shared(numMatrices, m, n, paddedN, src, dstPtr) schedule(static)
    for (long matrixIdx = 0; matrixIdx < static_cast<long>(numMatrices);
         matrixIdx++)
    {
        uint32_t nnz = 0;
        for (uint32_t rowIdx = 0; rowIdx < m; ++rowIdx)
        {
            for (uint32_t colIdx = 0; colIdx < n; ++colIdx)
                if (src[matrixIdx * m * paddedN + rowIdx * paddedN + colIdx] !=
                    0)
                    nnz++;
        }

        dstPtr[matrixIdx].ROW = static_cast<uint32_t*>(
            MemoryManager::GetMemoryHost(sizeof(uint32_t) * (m + 1)));
        dstPtr[matrixIdx].COL = static_cast<uint32_t*>(
            MemoryManager::GetMemoryHost(sizeof(uint32_t) * (!nnz ? 1 : nnz)));
        dstPtr[matrixIdx].V = static_cast<float*>(
            MemoryManager::GetMemoryHost(sizeof(uint32_t) * (!nnz ? 1 : nnz)));
        dstPtr[matrixIdx].M = m;
        dstPtr[matrixIdx].N = n;
        dstPtr[matrixIdx].NNZ = nnz;

        nnz = 0;
        for (uint32_t rowIdx = 0; rowIdx < m; ++rowIdx)
        {
            dstPtr[matrixIdx].ROW[rowIdx] = nnz;
            for (uint32_t colIdx = 0; colIdx < n; ++colIdx)
            {
                if (src[matrixIdx * m * paddedN + rowIdx * paddedN + colIdx] !=
                    0)
                {
                    dstPtr[matrixIdx].V[nnz] = src[matrixIdx * m * paddedN +
                                                   rowIdx * paddedN + colIdx];
                    dstPtr[matrixIdx].COL[nnz] = colIdx;
                    nnz++;
                }
            }
        }

        dstPtr[matrixIdx].ROW[m] = nnz;
    }
}

void ConvertSparseMatrixToDenseMatrix(float* dst, const SparseMatrix* src,
                                      uint32_t m, uint32_t n, uint32_t paddedN,
                                      uint32_t numMatrices)
{
#pragma omp parallel for default(none) shared(dst, numMatrices, m, n, paddedN)
    for (long matrixIdx = 0; matrixIdx < static_cast<long>(numMatrices); ++
         matrixIdx)
        for (long rowIdx = 0; rowIdx < static_cast<long>(m); ++rowIdx)
            for (long colIdx = 0; colIdx < static_cast<long>(n); ++colIdx)
                dst[matrixIdx * m * paddedN + rowIdx * paddedN + colIdx] = 0.0f;

#pragma omp parallel for default(none) \
    shared(src, dst, numMatrices, m, n, paddedN)
    for (long matrixIdx = 0; matrixIdx < static_cast<long>(numMatrices); ++
         matrixIdx)
    {
        const auto* rows = src[matrixIdx].ROW;
        const auto* cols = src[matrixIdx].COL;
        const auto* values = src[matrixIdx].V;

        for (uint32_t rowIdx = 0; rowIdx < m; ++rowIdx)
        {
            const auto sparseColIdxBegin = rows[rowIdx];
            const auto sparseColIdxEnd = rows[rowIdx + 1];
            for (uint32_t sparseColIdx = sparseColIdxBegin;
                 sparseColIdx < sparseColIdxEnd; ++sparseColIdx)
            {
                const auto colIdx = cols[sparseColIdx];
                const auto value = values[sparseColIdx];
                dst[matrixIdx * m * paddedN + rowIdx * paddedN + colIdx] =
                    value;
            }
        }
    }
}
} // namespace Sapphire::Compute
