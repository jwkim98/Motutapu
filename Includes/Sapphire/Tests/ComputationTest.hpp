// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef Sapphire_COMPUTATIONTEST_HPP
#define Sapphire_COMPUTATIONTEST_HPP

namespace Sapphire::Test
{
#ifdef WITH_CUDA
void Gemm1();

void Gemm2();

void GemmBroadcast();

void GemmBroadcastOnOutput();

#endif
}  // namespace Sapphire::Test

#endif  // Sapphire_COMPUTATIONTEST_HPP
