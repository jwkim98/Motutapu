// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef Sapphire_SPARSIFY_DECL_HPP
#define Sapphire_SPARSIFY_DECL_HPP

#include <Sapphire/util/SparseMatrix.hpp>

namespace Sapphire
{

template <typename T>
void DenseToSparse(T* dest, SparseMatrix<T> src);

template <typename T>
void SparseToDense(T* dest, SparseMatrix<T> src);

}

#endif
