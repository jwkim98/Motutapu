// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Motutapu/compute/cuda/Memory.cuh>
#include <Motutapu/tensor/TensorData.hpp>
#include <Motutapu/util/MemoryManager.hpp>
#include <algorithm>
#include <stdexcept>

namespace Motutapu::TensorUtil
{
TensorData::TensorData(Shape shape, Type type, Device device,
                       unsigned int batchSize)
    : BatchSize(batchSize),
      TensorShape(std::move(shape)),
      m_type(type),
      m_device(std::move(device))
{
    if (device.Type() == DeviceType::CUDA)
    {
        m_allocateCuda(batchSize);
        m_allocateCpu(batchSize);
    }
}

TensorData::~TensorData()
{
    m_freeHost();

    if (m_device.Type() == DeviceType::CUDA)
    {
        m_freeGpu();
    }
}

bool TensorData::CopyTensorData(TensorData dest, const TensorData src)
{
    if (src.GetDevice() != dest.GetDevice())
    {
        throw std::invalid_argument("Device mismatch while copying tensorData");
    }

    if (dest.TensorShape != src.TensorShape)
    {
        throw std::invalid_argument("Shape mismatch while copying tensorData");
    }

    if (dest.GetType() != src.GetType())
    {
        throw std::invalid_argument("Type mismatch while copying tensorData");
    }

    const bool sparse = src.GetType() == Type::Sparse;
    bool success = true;
    const auto device = src.GetDevice();

    const Shape shape = dest.TensorShape;

    if (device.Type() == DeviceType::CPU)
    {
        if (sparse)
        {
            throw std::runtime_error("CopyTensorData - sparse not implemented");
        }
        else
        {
            std::memcpy(dest.DenseMatHost, src.DenseMatHost,
                        src.DenseTotalLength * sizeof(float));
            dest.DenseTotalLength = src.DenseTotalLength;
        }
    }

    if (device.Type() == DeviceType::CUDA)
    {
        success &= Compute::Cuda::CudaSetDevice(device.GetID());

        if (sparse)
        {
            throw std::runtime_error("CopyTensorData - sparse not implemented");
        }
        else
        {
            Compute::Cuda::MemcpyGpuToGpu(dest.DenseMatCuda, src.DenseMatCuda,
                                          src.DenseTotalLength);
            dest.DenseTotalLength = src.DenseTotalLength;
        }
    }

    return success;
}

bool TensorData::SendTo(const Device &device)
{
    if (m_device == device)
    {
        return false;
    }

    if (m_device.Type() == DeviceType::CPU && device.Type() == DeviceType::CUDA)
    {
        m_device = device;
        m_allocateCuda(BatchSize);
        TensorData::m_toGpu(*this);
    }

    if (m_device.Type() == DeviceType::CUDA && device.Type() == DeviceType::CPU)
    {
        m_device = device;
        TensorData::m_toHost(*this);
    }

    if (m_device.Type() == DeviceType::CUDA &&
        device.Type() == DeviceType::CUDA)
    {
        TensorData::m_toHost(*this);
        m_device = device;
        TensorData::m_toGpu(*this);
    }

    return true;
}

void TensorData::m_toGpu(const TensorData &tensorData)
{
    if (tensorData.GetDevice().Type() != DeviceType::CUDA)
    {
        throw std::invalid_argument(
            "m_toGpu - Given tensor data is not GPU tensor");
    }

    if (tensorData.GetType() == Type::Sparse)
    {
        throw std::runtime_error("Sparse matrix not implemented");
    }
    else
    {
        Compute::Cuda::CudaSetDevice(tensorData.m_device.GetID());

        Compute::Cuda::MemcpyHostToGpu(tensorData.DenseMatCuda,
                                       tensorData.DenseMatHost,
                                       tensorData.BatchSize);
    }
}

void TensorData::m_toHost(const TensorData &tensorData)
{
    if (tensorData.GetDevice().Type() != DeviceType::CUDA)
    {
        throw std::invalid_argument(
            "m_toGpu - Given tensor data is not GPU tensor");
    }

    if (tensorData.GetType() == Type::Sparse)
    {
        throw std::runtime_error("Sparse matrix not implemented");
    }
    else
    {
        Compute::Cuda::CudaSetDevice(tensorData.m_device.GetID());
        Compute::Cuda::MemcpyGpuToHost(tensorData.DenseMatHost,
                                       tensorData.DenseMatCuda,
                                       tensorData.BatchSize);
    }
}

void TensorData::m_freeHost() const
{
    if (m_type == Type::Sparse)
    {
        delete[] SparseMatHost;
    }
    else
    {
        Util::MemoryManager::UnAssignMemoryHost(DenseMatHost);
    }
}

bool TensorData::m_freeGpu()
{
    bool isSuccess = true;

    isSuccess &= Compute::Cuda::CudaSetDevice(m_device.GetID());

    if (m_type == Type::Sparse)
    {
        isSuccess &=
            Compute::Cuda::CudaFree(reinterpret_cast<void *>(SparseMatCuda));
    }
    else
    {
        Util::MemoryManager::UnAssignMemoryCuda(DenseMatCuda, m_device.GetID());
    }

    return isSuccess;
}

void TensorData::m_allocateCpu(unsigned int batchSize)
{
    const auto colSize = TensorShape.At(0);
    const auto rowSize = TensorShape.Dim() > 1 ? TensorShape.At(1) : 1;

    const auto padUnitSize = static_cast<unsigned int>(32 / sizeof(float));

    const auto paddedColSize =
        colSize % padUnitSize == 0
            ? colSize
            : colSize / padUnitSize * padUnitSize + padUnitSize;

    const auto paddedRowSize =
        rowSize % padUnitSize == 0
            ? rowSize
            : rowSize / padUnitSize * padUnitSize + padUnitSize;

    if (m_type == Type::Sparse)
    {
        throw std::runtime_error("m_allocate - Sparse not implemented");
    }
    else
    {
        size_t totalSize = paddedColSize * paddedRowSize * batchSize;

        if (TensorShape.Dim() > 2)
        {
            for (auto i = 0; i < static_cast<int>(TensorShape.Dim()) - 1; ++i)
                totalSize *= TensorShape.At(i);
        }

        DenseTotalLength = totalSize;
        DenseMatHost = Util::MemoryManager::GetMemoryHost(totalSize);
    }
}

void TensorData::m_allocateCuda(unsigned int batchSize)
{
    if (m_device.Type() != DeviceType::CUDA)
    {
        throw std::runtime_error(
            "m_allocateCuda - Tensor Device type is not CUDA");
    }

    const auto colSize = TensorShape.At(0);
    const auto rowSize = TensorShape.Dim() > 1 ? TensorShape.At(1) : 1;

    const unsigned int padUnitSize = 32 / sizeof(float);

    const auto paddedColSize =
        colSize % padUnitSize == 0
            ? colSize
            : colSize / padUnitSize * padUnitSize + padUnitSize;

    const auto paddedRowSize =
        rowSize % padUnitSize == 0
            ? rowSize
            : rowSize / padUnitSize * padUnitSize + padUnitSize;

    if (m_type == Type::Sparse)
    {
        throw std::runtime_error("m_allocate - Sparse not implemented");
    }
    else
    {
        size_t totalSize = paddedColSize * paddedRowSize * batchSize;

        if (TensorShape.Dim() > 2)
        {
            for (auto i = 0; i < static_cast<int>(TensorShape.Dim()) - 1; ++i)
                totalSize *= TensorShape.At(i);
        }

        DenseTotalLength = totalSize;
        DenseMatCuda =
            Util::MemoryManager::GetMemoryCuda(totalSize, m_device.GetID());
    }
}

}  // namespace Motutapu::TensorUtil
