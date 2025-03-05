#pragma once

#include <miniaudio.h>

#include <span>
#include <functional>

namespace mute
{
    // struct AudioBuffer
    // {
    //     float* data = nullptr;
    //     std::size_t size = 0;
    //     std::size_t capacity = 0;
    //     bool own = false;

    //     AudioBuffer(float* data, std::size_t size, bool own)
    //         : data(data)
    //         , size(size)
    //         , capacity(size)
    //         , own(own) {}

    //     AudioBuffer(const AudioBuffer& other)
    //     {
    //         size = other.size;
    //         capacity = other.capacity;
    //         own = other.own;
    //         if (own)
    //         {
    //             data = (float*) malloc(capacity * sizeof(float));
    //             std::copy(other.begin(), other.end(), data);
    //         } else
    //             data = other.data;
    //     }

    //     AudioBuffer(AudioBuffer&& other)
    //     {
    //         std::swap(data, other.data);
    //         std::swap(size, other.size);
    //         std::swap(capacity, other.capacity);
    //         std::swap(own, other.own);
    //     }

    //     AudioBuffer& operator=(const AudioBuffer& other)
    //     {
    //         size = other.size;
    //         capacity = other.capacity;
    //         own = other.own;
    //         if (own)
    //         {
    //             data = (float*) malloc(capacity * sizeof(float));
    //             std::copy(other.begin(), other.end(), data);
    //         } else
    //             data = other.data;
    //         return *this;
    //     }

    //     AudioBuffer& operator=(AudioBuffer&& other)
    //     {
    //         std::swap(data, other.data);
    //         std::swap(size, other.size);
    //         std::swap(capacity, other.capacity);
    //         std::swap(own, other.own);
    //         return *this;
    //     }

    //     ~AudioBuffer()
    //     {
    //         if (own)
    //             free(data);
    //     }

    //     static AudioBuffer init(std::size_t size)
    //     {
    //         return AudioBuffer((float*) malloc(size * sizeof(float)), size, true);
    //     }
    //     static AudioBuffer copy(std::span<const float> source)
    //     {
    //         auto buffer = AudioBuffer::init(source.size());
    //         std::ranges::copy(source, buffer.data);
    //         return buffer;
    //     }
    //     static AudioBuffer ref(std::span<float> source)
    //     {
    //         return AudioBuffer(source.data(), source.size(), false);
    //     }
    //     static AudioBuffer ref(float* data, size_t size) { return AudioBuffer(data, size, false); }
    //     static AudioBuffer ref(std::span<const float> source)
    //     {
    //         return AudioBuffer((float*) source.data(), source.size(), false);
    //     }
    //     static AudioBuffer ref(AudioBuffer& source)
    //     {
    //         return AudioBuffer(source.data, source.size, false);
    //     }
    //     static AudioBuffer ref(const AudioBuffer& source)
    //     {
    //         return AudioBuffer((float*) source.data, source.size, false);
    //     }

    //     float* begin() { return data; }
    //     float* end() { return data + size; }
    //     const float* begin() const { return data; }
    //     const float* end() const { return data + size; }

    //     float& operator[](std::size_t index) { return data[index]; }
    //     const float& operator[](std::size_t index) const { return data[index]; }

    //     void resize(std::size_t ns)
    //     {
    //         size = ns;
    //         if (own && data && size > capacity)
    //         {
    //             data = (float*) realloc(data, size);
    //             capacity = size;
    //         }
    //     }

    //     AudioBuffer& operator+=(const AudioBuffer& right) { for (std::size_t i = 0; i < size && i < right.size; i++) data[i] += right[i]; return *this; }
    //     AudioBuffer& operator-=(const AudioBuffer& right) { for (std::size_t i = 0; i < size && i < right.size; i++) data[i] -= right[i]; return *this; }
    //     AudioBuffer& operator*=(const AudioBuffer& right) { for (std::size_t i = 0; i < size && i < right.size; i++) data[i] *= right[i]; return *this; }
    //     AudioBuffer& operator/=(const AudioBuffer& right) { for (std::size_t i = 0; i < size && i < right.size; i++) data[i] /= right[i]; return *this; }

    //     AudioBuffer& operator+=(float uniform) { for (std::size_t i = 0; i < size; i++) data[i] += uniform; return *this; }
    //     AudioBuffer& operator-=(float uniform) { for (std::size_t i = 0; i < size; i++) data[i] -= uniform; return *this; }
    //     AudioBuffer& operator*=(float uniform) { for (std::size_t i = 0; i < size; i++) data[i] *= uniform; return *this; }
    //     AudioBuffer& operator/=(float uniform) { for (std::size_t i = 0; i < size; i++) data[i] /= uniform; return *this; }
    // };

    struct AudioProcessData
    {
        struct {
            std::span<const float> buffer;
            int channels;
        } capture;
        struct {
            std::span<float> buffer;
            int channels;
        } playback;
        int frames;
        int sampleRate;
    };

    using AudioCallback = std::function<void(AudioProcessData data)>;

    struct AudioDriverConfiguration
    {
        struct {
            int channels = 0;
            std::string deviceName;
        } capture;
        struct {
            int channels = 0;
            std::string deviceName;
        } playback;
        int sampleRate = -1;
        int bufferSize = -1;
        AudioCallback callback;
    };

    struct AudioDriver
    {
        static constexpr int DefaultSampleRate = 48000;
        static constexpr int DefaultBufferSize = 128;

        ma_context context;
        ma_device device;

        AudioDriverConfiguration configuration;

        AudioDriver(const AudioDriverConfiguration& config = {});
        ~AudioDriver();

        bool valid() const;
        void internalAudioCallback(const float* input, float* output, int frames);
        void start();
        void stop();
    };
}