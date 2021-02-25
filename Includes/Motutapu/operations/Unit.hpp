// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef MOTUTAPU_UNIT_DECL_HPP
#define MOTUTAPU_UNIT_DECL_HPP

#include <Motutapu/compute/cuda/dense/Convolution.cuh>
#include <Motutapu/tensor/TensorData.hpp>
#include <unordered_map>

namespace Motutapu
{
class UnitDataWrapper
{
 public:
    UnitDataWrapper() = default;
    virtual ~UnitDataWrapper() = default;

    UnitDataWrapper(const UnitDataWrapper& unit) = default;
    UnitDataWrapper(UnitDataWrapper&& unitDataWrapper) noexcept = default;
    UnitDataWrapper& operator=(const UnitDataWrapper& unit) = default;
    UnitDataWrapper& operator=(UnitDataWrapper&& unitDataWrapper) noexcept =
        default;

    std::unordered_map<std::string, TensorUtil::TensorData> TensorDataMap;
    std::unordered_map<std::string, std::string> StringLiterals;
    std::unordered_map<std::string, float> ScalarLiterals;
    std::unordered_map<std::string, int> IntegerLiterals;

    Compute::Cuda::CudnnMetaData CudnnConvMetaData;

    std::string Name;
    Device HostDevice;
    int Key = -1;
};
}  // namespace Motutapu

#endif
