// #pragma once

// #include <complex>
// #include <span>

// #include <fftw3.h>

// namespace mute
// {
//     // struct IFFT {
//     //     // Bit-reversal for FFT input reordering
//     //     unsigned int bitReverse(unsigned int x, int bits) {
//     //         unsigned int result = 0;
//     //         for (int i = 0; i < bits; i++) {
//     //             result = (result << 1) | (x & 1);
//     //             x >>= 1;
//     //         }
//     //         return result;
//     //     }

//     //     // Check if number is power of 2
//     //     bool isPowerOfTwo(size_t x) {
//     //         return x && !(x & (x - 1));
//     //     }

//     //     // Calculate number of bits needed to represent a number
//     //     int numBits(size_t x) {
//     //         int bits = 0;
//     //         while (x > 0) {
//     //             bits++;
//     //             x >>= 1;
//     //         }
//     //         return bits;
//     //     }

//     //     // Main IFFT function
//     //     void compute(std::span<const std::complex<float>> input, std::span<std::complex<float>> output) {
//     //         size_t n = input.size();
            
//     //         // Verify input size is power of 2
//     //         if (!isPowerOfTwo(n)) {
//     //             throw std::invalid_argument("Input size must be a power of 2");
//     //         }

//     //         int bits = numBits(n) - 1;

//     //         // Bit reversal permutation
//     //         for (size_t i = 0; i < n; i++) {
//     //             size_t rev = bitReverse(i, bits);
//     //             if (i < rev) {
//     //                 std::swap(output[i], output[rev]);
//     //             }
//     //         }

//     //         // Inverse FFT computation using butterfly operations
//     //         for (size_t step = 1; step < n; step <<= 1) {
//     //             float theta = M_PI / step;  // Note: M_PI instead of -M_PI for IFFT
//     //             std::complex<float> wn(cos(theta), sin(theta));
                
//     //             for (size_t i = 0; i < n; i += 2 * step) {
//     //                 std::complex<float> w(1, 0);
                    
//     //                 for (size_t j = 0; j < step; j++) {
//     //                     std::complex<float> u = output[i + j];
//     //                     std::complex<float> t = w * output[i + j + step];
                        
//     //                     output[i + j] = u + t;
//     //                     output[i + j + step] = u - t;
                        
//     //                     w *= wn;
//     //                 }
//     //             }
//     //         }

//     //         // Scale the output
//     //         for (auto& x : output) {
//     //             x /= n;
//     //         }
//     //     }
//     // };



//     struct FFT
//     {
//         size_t size = 0;
//         fftwf_complex *fftin, *fftout;
//         fftwf_plan plan;

//         ~FFT()
//         {
//             if (size != 0)
//             {
//                 fftwf_destroy_plan(plan);
//                 fftwf_free(fftin);
//                 fftwf_free(fftout);
//             }
//         }

//         void init(size_t size)
//         {
//             this->size = size;
//             fftin = (fftwf_complex*) fftwf_malloc(size * sizeof(fftwf_complex));
//             fftout = (fftwf_complex*) fftwf_malloc(size * sizeof(fftwf_complex));
//             plan = fftwf_plan_dft_1d(size, fftin, fftout, FFTW_FORWARD, FFTW_PATIENT);
//         }

//         void compute(std::span<const float> in, std::span<std::complex<float>> out)
//         {
//             for (size_t i = 0; i < in.size() && i < size; i++)
//             {
//                 fftin[i][0] = in[i];
//                 fftin[i][1] = 0.0;
//             }

//             fftwf_execute(plan);

//             for (size_t i = 0; i < out.size() && i < size; i++)
//                 out[i] = { fftout[i][0], fftout[i][1] };
//         }

//         void compute(std::span<const float> in, std::span<float> out)
//         {
//             for (size_t i = 0; i < in.size() && i < size; i++)
//             {
//                 fftin[i][0] = in[i];
//                 fftin[i][1] = 0.0;
//             }

//             fftwf_execute(plan);

//             for (size_t i = 0; i < out.size() && i < size; i++)
//                 out[i] = std::sqrt(fftout[i][0] * fftout[i][0] + fftout[i][1] * fftout[i][1]);
//         }
//     };
// }