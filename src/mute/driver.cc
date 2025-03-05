#include "mute/driver.h"
#include "mute/debug.h"

#include <span>
#include <ranges>
#include <fmt/format.h>
#include <fmt/ranges.h>

namespace mute
{
    AudioDriver::AudioDriver(const AudioDriverConfiguration& config)
        : configuration(config)
    {
        ma_result result = MA_SUCCESS;

        if ((result = ma_context_init(nullptr, 0, nullptr, &context)))
        {
            log::error("Error initializing context");
            return;
        }

        ma_device_type deviceType = {};
        if (config.capture.channels > 0) deviceType = ma_device_type_capture;
        if (config.playback.channels > 0) deviceType = ma_device_type(deviceType | ma_device_type_playback);
        auto deviceConfig = ma_device_config_init(deviceType);
        
        ma_device_info* captureDevices;
        ma_device_info* playbackDevices;
        uint32_t captureDevicesCount;
        uint32_t playbackDevicesCount;
        if ((result = ma_context_get_devices(&context
            , &playbackDevices, &playbackDevicesCount
            , &captureDevices, &captureDevicesCount)))
        {
            log::error("Error getting devices list");
            return;
        }

        auto captureDevicesNames = std::span(captureDevices, captureDevicesCount)
            | std::views::transform([](auto dev) { return std::string(dev.name); });
        log::info(
            fmt::format("Available audio capture devices:\n\t- {}"
                , fmt::join(captureDevicesNames, "\n\t- "))
        );

        auto playbackDevicesNames = std::span(playbackDevices, playbackDevicesCount)
            | std::views::transform([](auto dev) { return std::string(dev.name); });
        log::info(
            fmt::format("Available audio playback devices:\n\t- {}"
                , fmt::join(playbackDevicesNames, "\n\t- "))
        );

        deviceConfig.capture.channels = config.capture.channels;
        deviceConfig.capture.format = ma_format_f32;
        for (auto i = 0u; i < captureDevicesCount; i++)
        {
            if (config.capture.channels > 0 && config.capture.deviceName == captureDevices[i].name)
            {
                log::info(fmt::format("* Found requested capture device {}", captureDevices[i].name));
                deviceConfig.capture.pDeviceID = &captureDevices[i].id;
            }
        }

        deviceConfig.playback.channels = config.playback.channels;
        deviceConfig.playback.format = ma_format_f32;
        for (auto i = 0u; i < playbackDevicesCount; i++)
        {
            if (config.playback.channels > 0 && config.playback.deviceName == playbackDevices[i].name)
            {
                log::info(fmt::format("* Found requested playback device {}", playbackDevices[i].name));
                deviceConfig.playback.pDeviceID = &playbackDevices[i].id;
            }
        }

        deviceConfig.periods = 1;
        deviceConfig.periodSizeInFrames = config.bufferSize > 0
            ? config.bufferSize
            : DefaultBufferSize;
        deviceConfig.sampleRate = config.sampleRate > 0
            ? config.sampleRate
            : DefaultSampleRate;

        deviceConfig.pUserData = this;
        deviceConfig.dataCallback = [](ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
        {
            auto& context = *(AudioDriver*) pDevice->pUserData;
            context.internalAudioCallback((const float*) pInput, (float*) pOutput, frameCount);
        };

        if ((result = ma_device_init(&context, &deviceConfig, &device)))
        {
            log::error("Error initializing device");
            return;
        }
    }

    AudioDriver::~AudioDriver()
    {
        stop();
        ma_context_uninit(&context);
    }

    bool AudioDriver::valid() const
    {
        return device.state.value  != ma_device_state_uninitialized;
    }

    void AudioDriver::internalAudioCallback(const float* input, float* output, int frames)
    {
        configuration.callback(AudioProcessData {
            .capture = {
                .buffer = std::span(input, configuration.capture.channels * frames),
                .channels = configuration.capture.channels
            },
            .playback = {
                .buffer = std::span(output, configuration.playback.channels * frames),
                .channels = configuration.playback.channels
            },
            .frames = frames,
            .sampleRate = configuration.sampleRate,
        });
    }

    void AudioDriver::start()
    {
        if (!ma_device_is_started(&device))
            ma_device_start(&device);
    }
    void AudioDriver::stop()
    {
        if (ma_device_is_started(&device))
            ma_device_stop(&device);
    }
}