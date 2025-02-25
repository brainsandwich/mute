#pragma once

#include <complex>
#include <span>

namespace mute
{
    struct IFFT {
        // Bit-reversal for FFT input reordering
        unsigned int bitReverse(unsigned int x, int bits) {
            unsigned int result = 0;
            for (int i = 0; i < bits; i++) {
                result = (result << 1) | (x & 1);
                x >>= 1;
            }
            return result;
        }

        // Check if number is power of 2
        bool isPowerOfTwo(size_t x) {
            return x && !(x & (x - 1));
        }

        // Calculate number of bits needed to represent a number
        int numBits(size_t x) {
            int bits = 0;
            while (x > 0) {
                bits++;
                x >>= 1;
            }
            return bits;
        }

        // Main IFFT function
        void compute(std::span<std::complex<float>> input, std::span<std::complex<float>> output) {
            size_t n = input.size();
            
            // Verify input size is power of 2
            if (!isPowerOfTwo(n)) {
                throw std::invalid_argument("Input size must be a power of 2");
            }

            int bits = numBits(n) - 1;

            // Bit reversal permutation
            for (size_t i = 0; i < n; i++) {
                size_t rev = bitReverse(i, bits);
                if (i < rev) {
                    std::swap(output[i], output[rev]);
                }
            }

            // Inverse FFT computation using butterfly operations
            for (size_t step = 1; step < n; step <<= 1) {
                float theta = M_PI / step;  // Note: M_PI instead of -M_PI for IFFT
                std::complex<float> wn(cos(theta), sin(theta));
                
                for (size_t i = 0; i < n; i += 2 * step) {
                    std::complex<float> w(1, 0);
                    
                    for (size_t j = 0; j < step; j++) {
                        std::complex<float> u = output[i + j];
                        std::complex<float> t = w * output[i + j + step];
                        
                        output[i + j] = u + t;
                        output[i + j + step] = u - t;
                        
                        w *= wn;
                    }
                }
            }

            // Scale the output
            for (auto& x : output) {
                x /= n;
            }
        }
    };
}