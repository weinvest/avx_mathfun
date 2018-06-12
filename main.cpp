#include <gtest/gtest.h>
#include <random>
#include <cmath>
#include <iostream>
#include "avx_mathfun.h"

constexpr size_t VECTOR_LEN = 1UL<<16;
constexpr size_t ALIGN = 32;
constexpr size_t ITER_COUNT = 10000;
constexpr float EPSILON = 1e-5;

class AVXMathfunTest : public ::testing::Test {
private:
    template <typename T>
    inline void allocMemory(T** data) {
        posix_memalign(reinterpret_cast<void**>(data), ALIGN, VECTOR_LEN*sizeof(float));
    }

#define allocMem(func)\
    do {\
        allocMemory(& func##NaiveResult_);\
        allocMemory(& func##SimdResult_);\
        allocMemory(& me_##func##SimdResult_);\
        allocMemory(& func##Result_);\
    } while(0)

#define freeMem(func)\
    do {\
        free(func##NaiveResult_);\
        free(func##SimdResult_);\
        free(me_##func##SimdResult_);\
        free(func##Result_);\
    } while(0)


protected:
    virtual void TearDown() {
        free(inputData_);
        freeMem(exp);
        freeMem(log);
        freeMem(sin);
        freeMem(cos);
    }

    virtual void SetUp() {
        allocMemory(&inputData_);
        allocMem(exp);
        allocMem(log);
        allocMem(sin);
        allocMem(cos);


        std::random_device dev;
        std::mt19937_64 eng;
        eng.seed(dev());
        std::uniform_real_distribution<float> distribution(0, 1);

        for (size_t i = 0; i < VECTOR_LEN; ++i) {
            float tmp = distribution(eng);
            inputData_[i] = tmp;
            expNaiveResult_[i] = std::exp(tmp);
            logNaiveResult_[i] = std::log(tmp);
            sinNaiveResult_[i] = std::sin(tmp);
            cosNaiveResult_[i] = std::cos(tmp);
        }
    }

    float* inputData_;
    float* expNaiveResult_;
    float* expSimdResult_;
    float* logNaiveResult_;
    float* logSimdResult_;
    float* sinNaiveResult_;
    float* sinSimdResult_;
    float* cosNaiveResult_;
    float* cosSimdResult_;

    float* expResult_;
    float* logResult_;
    float* sinResult_;
    float* cosResult_;

    float* me_expSimdResult_;
    float* me_logSimdResult_;
    float* me_sinSimdResult_;
    float* me_cosSimdResult_;
};

#define TEST_AVX(func)\
TEST_F(AVXMathfunTest, func) {\
    for (size_t i = 0; i < ITER_COUNT; ++i) {\
        __m256 tmp;\
        __m256 ipt;\
        for (size_t j = 0; j < VECTOR_LEN; j+= 8) {\
            ipt = _mm256_load_ps(inputData_ + j);\
            tmp = func##256_ps(ipt);\
            _mm256_store_ps(func##SimdResult_ + j, tmp);\
        }\
    }\
\
    for (size_t i = 0; i < VECTOR_LEN; ++i) {\
        ASSERT_NEAR(func##NaiveResult_[i], func##SimdResult_[i], EPSILON);\
    }\
}

#define TEST_ME_AVX(func)\
TEST_F(AVXMathfunTest, me_##func) {\
    for (size_t i = 0; i < ITER_COUNT; ++i) {\
        __m256 tmp;\
        __m256 ipt;\
        for (size_t j = 0; j < VECTOR_LEN; j+= 8) {\
            ipt = _mm256_load_ps(inputData_ + j);\
            tmp = me_##func##256_ps(ipt);\
            _mm256_store_ps(me_##func##SimdResult_ + j, tmp);\
        }\
    }\
\
    for (size_t i = 0; i < VECTOR_LEN; ++i) {\
        ASSERT_NEAR(func##NaiveResult_[i], me_##func##SimdResult_[i], EPSILON);\
    }\
}

#define TEST_NAIVE(func)\
TEST_F(AVXMathfunTest, naive_##func) {\
    for (size_t i = 0; i < ITER_COUNT; ++i) {\
        float ipt;\
        float tmp;\
        for (size_t j = 0; j < VECTOR_LEN; j+= 1) {\
            ipt = inputData_[j];\
            tmp = std::func(ipt);\
            func##Result_[j] = tmp;\
        }\
    }\
\
    for (size_t i = 0; i < VECTOR_LEN; ++i) {\
        ASSERT_NEAR(func##NaiveResult_[i], func##Result_[i], EPSILON);\
    }\
}
TEST_AVX(exp)
TEST_ME_AVX(exp)
TEST_NAIVE(exp)

TEST_AVX(log)
TEST_ME_AVX(log)
TEST_NAIVE(log)

TEST_AVX(sin)
TEST_NAIVE(sin)
TEST_AVX(cos)
TEST_NAIVE(cos)
