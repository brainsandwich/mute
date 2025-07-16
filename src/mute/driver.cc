#include "mute/driver.h"
#include "mute/debug.h"

#include <span>
#include <ranges>
#include <fmt/format.h>
#include <fmt/ranges.h>

namespace mute
{
    bool Driver::init()
    {
        ma_result result = MA_SUCCESS;

        if ((result = ma_context_init(nullptr, 0, nullptr, &ma.context)))
        {
            log::error("Error initializing context");
            return false;
        }

        ma_device_type deviceType = {};
        if (configuration.capture) deviceType = ma_device_type_capture;
        if (configuration.playback) deviceType = ma_device_type(deviceType | ma_device_type_playback);
        auto deviceConfig = ma_device_config_init(deviceType);
        
        ma_device_info* captureDevices;
        ma_device_info* playbackDevices;
        uint32_t captureDevicesCount;
        uint32_t playbackDevicesCount;
        if ((result = ma_context_get_devices(&ma.context
            , &playbackDevices, &playbackDevicesCount
            , &captureDevices, &captureDevicesCount)))
        {
            log::error("Error getting devices list");
            return false;
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

        if (configuration.capture)
        {
            auto captureConfig = *configuration.capture;
            deviceConfig.capture.channels = captureConfig.channels;
            deviceConfig.capture.format = ma_format_f32;
            for (auto i = 0u; i < captureDevicesCount; i++)
            {
                if (captureConfig.channels > 0 && captureConfig.device == captureDevices[i].name)
                {
                    log::info(fmt::format("* Found requested capture device {}", captureDevices[i].name));
                    deviceConfig.capture.pDeviceID = &captureDevices[i].id;
                }
            }
        }

        if (configuration.playback)
        {
            auto playbackConfig = *configuration.playback;
            deviceConfig.playback.channels = playbackConfig.channels;
            deviceConfig.playback.format = ma_format_f32;
            for (auto i = 0u; i < playbackDevicesCount; i++)
            {
                if (playbackConfig.channels > 0 && playbackConfig.device == playbackDevices[i].name)
                {
                    log::info(fmt::format("* Found requested playback device {}", playbackDevices[i].name));
                    deviceConfig.playback.pDeviceID = &playbackDevices[i].id;
                }
            }
        }

        deviceConfig.periods = 1;
        deviceConfig.periodSizeInFrames = configuration.bufferSize > 0
            ? configuration.bufferSize
            : DefaultBufferSize;
        deviceConfig.sampleRate = configuration.sampleRate > 0
            ? configuration.sampleRate
            : DefaultSampleRate;

        deviceConfig.pUserData = this;
        deviceConfig.dataCallback = [](ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
        {
            auto& context = *(Driver*) pDevice->pUserData;
            context.internalAudioCallback((const float*) pInput, (float*) pOutput, frameCount);
        };

        if ((result = ma_device_init(&ma.context, &deviceConfig, &ma.device)))
        {
            log::error("Error initializing device");
            return false;
        }
        
        return true;
    }

    void Driver::uninit()
    {
        stop();
        if (ma_device_get_state(&ma.device) != ma_device_state_uninitialized)
            ma_device_uninit(&ma.device);
        ma_context_uninit(&ma.context);
        ma.context = {};
    }

    bool Driver::valid() const
    {
        return ma_device_get_state(&ma.device) != ma_device_state_uninitialized;
    }

    void Driver::internalAudioCallback(const float* input, float* output, int frames)
    {
        audioCallback(ProcessContext {
            .capture = {
                .buffer = std::span(input, configuration.capture ? configuration.capture->channels * frames : 0),
                .channels = configuration.capture ? configuration.capture->channels : 0
            },
            .playback = {
                .buffer = std::span(output, configuration.playback ? configuration.playback->channels * frames : 0),
                .channels = configuration.playback ? configuration.playback->channels : 0
            },
            .frames = frames,
            .sampleRate = configuration.sampleRate,
        });
    }

    void Driver::start()
    {
        if (!ma_device_is_started(&ma.device))
            ma_device_start(&ma.device);
    }

    void Driver::stop()
    {
        if (ma_device_is_started(&ma.device))
            ma_device_stop(&ma.device);
    }
}