// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef Sapphire_UTIL_TENSORDESCRIPTOR_DECL_HPP
#define Sapphire_UTIL_TENSORDESCRIPTOR_DECL_HPP

#include <Sapphire/operations/Backward/BackPropWrapper.hpp>
#include <Sapphire/tensor/TensorData.hpp>
#include <list>
#include <memory>
#include <mutex>

namespace Sapphire::TensorUtil
{
//! TensorDescriptor stores real tensor data of the tensor
//! There can be more than one tensor that references to one tensorData
//! All public functions in the TensorDescriptor is guaranteed to be thread safe
//! TensorDescriptor should not be accessible from the user interface directly
class TensorDescriptor
{
 public:
    TensorDescriptor() = default;

    TensorDescriptor(const Shape& shape, Type type, const Device& device,
                     unsigned int batchSize, int key);

    ~TensorDescriptor() = default;

    TensorDescriptor(const TensorDescriptor& tensorData) = delete;
    TensorDescriptor(TensorDescriptor&& tensorData) noexcept;
    TensorDescriptor& operator=(const TensorDescriptor& tensorData) = delete;
    TensorDescriptor& operator=(TensorDescriptor&& tensorDesc) noexcept;

    TensorData ForwardData;
    TensorData BackwardData;

    //! Add unit m_key if unit was used as output or flow-through type
    //! \param wrapper : Wrapper for starting back propagation on this tensor
    //! \param saveOutput : Forward output of this tensorDescriptor is preserved
    //! if true
    void AppendOutputHistory(std::unique_ptr<BackProp::BackPropWrapper> wrapper,
                             bool saveOutput);

    //! Add unit key if unit was used as operand only
    //! \param tensorKey : m_key of the tensor that this tensor should receive
    //! gradient from
    void AppendOperandHistory(int tensorKey);

    void RemoveGradientInputKey(int tensorKey);

    //! Removes last history from the history list if it is operand history
    void PopIfOperandHistory();

    void PopHistory();

    //! Returns whether this tensorDescriptor is trainable
    //! \return : True if gradient is required false otherwise
    [[nodiscard]] bool IsTrainable() const
    {
        return m_trainable;
    }

    //! Checks if next operation is output unit in back propagation
    //! \return : true if ready false otherwise
    [[nodiscard]] bool IsBackPropReady() const;

    const std::unique_ptr<BackProp::BackPropWrapper>& GetBackPropWrapper()
    {
        return m_history.back().Wrapper;
    }

    [[nodiscard]] int GetKey() const
    {
        return m_key;
    }

    [[nodiscard]] unsigned int GetBatchSize() const
    {
        return m_batchSize;
    }

    // todo : Create sendTo, GetDevice, GetType

 private:
    //! This describes history of the tensorData
    //! As tensorData is used in unit function as an operand or input/output.
    //! It is stored using this struct
    struct History
    {
        explicit History(std::unique_ptr<BackProp::BackPropWrapper> wrapper)
            : IsOutput(true), Wrapper(std::move(wrapper))
        {
        }

        History() : IsOutput(false)
        {
        }

        History(History&& history) noexcept = default;
        History(const History& history) = delete;
        History& operator=(History&& history) noexcept = default;
        History& operator=(const History& history) = delete;

        void AddGradientInputTensorKey(int key)
        {
            GradientInputTensorKeys.emplace_back(key);
        }

        bool IsOutput;

        std::unique_ptr<BackProp::BackPropWrapper> Wrapper;
        //! List of the units that was as operand
        std::list<int> GradientInputTensorKeys;
    };

    //! m_key to identify tensor data
    int m_key = -1;
    unsigned int m_batchSize;
    bool m_trainable = true;

    std::list<History> m_history;
};
}  // namespace Sapphire::TensorUtil

#endif
