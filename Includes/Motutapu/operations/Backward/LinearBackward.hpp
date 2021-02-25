// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef MOTUTAPU_LINEARBACKWARD_HPP
#define MOTUTAPU_LINEARBACKWARD_HPP

#include <Motutapu/Model.hpp>
#include <Motutapu/compute/Compute.hpp>
#include <Motutapu/compute/Initialize.hpp>
#include <Motutapu/operations/Backward/BackPropWrapper.hpp>

namespace Motutapu::BackProp
{
class LinearBackProp : public BackPropWrapper
{
 public:
    explicit LinearBackProp(const TensorUtil::TensorData& x, TensorUtil::TensorData dx,
                            TensorUtil::TensorData dy, int unitKey);

    bool InvokeBackProp(const TensorUtil::TensorData& input) override;

 private:
    void m_backProp(const TensorUtil::TensorData& weight);

    void m_updateWeight(TensorUtil::TensorData& weight);

    void m_updateBias(TensorUtil::TensorData& bias);

    unsigned int m_batchSize;
};

}  // namespace Motutapu::BackProp

#endif  // MOTUTAPU_LINEARBACKWARD_HPP
