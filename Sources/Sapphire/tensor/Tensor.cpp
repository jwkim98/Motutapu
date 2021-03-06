// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Sapphire/Model.hpp>
#include <Sapphire/tensor/Tensor.hpp>

namespace Sapphire
{
Tensor::Tensor(Shape shape, unsigned int descKey)
    : m_shape(std::move(shape)), m_tensorDescriptorKey(descKey)
{
}

Tensor& Tensor::operator=(const Tensor& tensor)
{
    if (&tensor == this)
        return *this;

    m_shape = tensor.m_shape;
    m_tensorDescriptorKey = tensor.m_tensorDescriptorKey;
    return *this;
}

Shape Tensor::GetShape() const
{
    return m_shape;
}

Device Tensor::GetDevice() const
{
    Model& model = ModelManager::GetCurrentModel();
    TensorUtil::TensorDescriptor& desc = model.GetDescriptor(m_tensorDescriptorKey);
    return desc.ForwardData.GetDevice();
}

int Tensor::TensorDescriptorKey() const
{
    return m_tensorDescriptorKey;
}

void Tensor::SendTo(const Device& device) const
{
    Model& model = ModelManager::GetCurrentModel();
    TensorUtil::TensorDescriptor& desc = model.GetDescriptor(m_tensorDescriptorKey);
    desc.ForwardData.SendTo(device);
    if (desc.IsTrainable())
    {
        desc.BackwardData.SendTo(device);
    }
}

}  // namespace Sapphire
