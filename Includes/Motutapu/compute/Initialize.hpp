// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef MOTUTAPU_INITIALIZE_HPP
#define MOTUTAPU_INITIALIZE_HPP

#include <Motutapu/tensor/TensorData.hpp>

namespace Motutapu::Compute
{
void NormalFloat(TensorUtil::TensorData data, float mean, float sd,
                        unsigned int size);

void Ones(TensorUtil::TensorData data);

void Zeros(TensorUtil::TensorData data);

void HeNormal(TensorUtil::TensorData data, int fanIn);

void Xavier(TensorUtil::TensorData data, int fanIn, int fanOut);

}  // namespace Motutapu::Compute

#endif  // MOTUTAPU_INITIALIZE_HPP