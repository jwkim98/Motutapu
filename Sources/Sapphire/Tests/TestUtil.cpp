// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Sapphire/Tests/TestUtil.hpp>
#include <random>

namespace Sapphire::Test
{
void InitIntegerDenseMatrix(float* matrixPtr, const size_t m, const size_t n,
                            const size_t paddedN, const size_t numMatrices,
                            const float sparsity)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    std::uniform_int_distribution<int> uniform(-30, 30);
    std::normal_distribution<float> normal(0, 10);

    for (long matrixIdx = 0; matrixIdx < static_cast<long>(numMatrices); ++
         matrixIdx)
        for (long rowIdx = 0; rowIdx < static_cast<long>(m); ++rowIdx)
            for (long colIdx = 0; colIdx < static_cast<long>(n); ++colIdx)
            {
                if (prob(gen) > static_cast<double>(sparsity))
                    matrixPtr[matrixIdx * m * paddedN + rowIdx * paddedN +
                              colIdx] = static_cast<float>(uniform(gen));
                else
                    matrixPtr[matrixIdx * m * paddedN + rowIdx * paddedN +
                              colIdx] = 0.0f;
            }
}

void InitRandomDenseMatrix(float* matrixPtr, const size_t m, const size_t n,
                           const size_t paddedN, const size_t numMatrices,
                           const float sparsity)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    std::normal_distribution<float> normal(0, 10);

    for (long matrixIdx = 0; matrixIdx < static_cast<long>(numMatrices); ++
         matrixIdx)
        for (long rowIdx = 0; rowIdx < static_cast<long>(m); ++rowIdx)
            for (long colIdx = 0; colIdx < static_cast<long>(n); ++colIdx)
            {
                if (uniform(gen) > static_cast<double>(sparsity))
                    matrixPtr[matrixIdx * m * paddedN + rowIdx * paddedN +
                              colIdx] = normal(gen);
                else
                    matrixPtr[matrixIdx * m * paddedN + rowIdx * paddedN +
                              colIdx] = 0.0f;
            }
}
} // namespace Sapphire::Test
