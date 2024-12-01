#pragma once

#include "openfhe.h"

using namespace lbcrypto;

class Practice {
    public:
        static Ciphertext<lbcrypto::DCRTPoly>
        evaluateLinearPolynomial(const CryptoContext<DCRTPoly> cc,
                                const Ciphertext<lbcrypto::DCRTPoly> x,
                                const Ciphertext<lbcrypto::DCRTPoly> y);

        static Ciphertext<DCRTPolyImpl<BigVector>>
        computeTriangleArea(const CryptoContext<DCRTPoly> cc,
                            const Ciphertext<lbcrypto::DCRTPoly> base,
                            const Ciphertext<lbcrypto::DCRTPoly> height);

        static Ciphertext<lbcrypto::DCRTPoly>
        evaluateCubicPolynomial(const CryptoContext<DCRTPoly> cc,
                                const Ciphertext<lbcrypto::DCRTPoly> x);

        static std::vector<double>
        decrypt_and_decode(const CryptoContext<DCRTPoly> cc,
                            const Ciphertext<DCRTPoly> &ciphertext,
                            const PrivateKey<DCRTPoly> &sk);

        static Ciphertext<DCRTPoly>
        computeAverage(const CryptoContext<DCRTPoly> cc,
                    const Ciphertext<DCRTPoly> vector);

        static Ciphertext<DCRTPoly>
        computeDotProduct(const CryptoContext<DCRTPoly> cc,
                        const Ciphertext<DCRTPoly> vector1,
                        const Ciphertext<DCRTPoly> vector2);

        static std::vector<double>
        decrypt_and_decode(const CryptoContext<DCRTPoly> &cc,
                        const Ciphertext<DCRTPoly> &ciphertext,
                        const PrivateKey<DCRTPoly> &sk);

        static Ciphertext<DCRTPoly> 
        customRotate(const CryptoContext<DCRTPoly> cc,
                    const Ciphertext<DCRTPoly> input,
                    int32_t index);

        static Ciphertext<DCRTPoly>
        permutateVector(const CryptoContext<DCRTPoly> cc,
                        const Ciphertext<DCRTPoly> input);

        static Ciphertext<DCRTPoly>
        efficientCubicPolynomial(const CryptoContext<DCRTPoly> cc,
                                const Ciphertext<DCRTPoly> x);

        static std::vector<double>
        decrypt_and_decode(const CryptoContext<DCRTPoly> &cc,
                        const Ciphertext<DCRTPoly> &ciphertext,
                        const PrivateKey<DCRTPoly> &sk);
        
};