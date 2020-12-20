#ifndef _ECHIDNA_CLIENT_RENDERER_HPP
#define _ECHIDNA_CLIENT_RENDERER_HPP

#include "client/device.hpp"
#include "client/clutil.hpp"
#include "client/rendertask.hpp"
#include "utils/threadpool.hpp"

#include <CL/cl.h>
#include <vector>
#include <array>
#include <condition_variable>
#include <optional>
#include <cstdint>

namespace echidna::client {
    struct DeviceTaskInfo {
        Device* device;
        UniqueKernel kernel;
        std::vector<UniqueMemObject> render_targets;
        std::vector<std::vector<uint8_t>> host_render_targets;
    };

    struct RenderTask {
        RenderTaskInfo task_info;
        std::vector<DeviceTaskInfo> device_info;
    };

    class Renderer {
        private:
            constexpr static const cl_image_format RENDER_TARGET_FORMAT = {CL_RGBA, CL_UNORM_INT8};

            utils::ThreadPool compression_pool;
            std::vector<Device> devices;
            std::mutex mutex;
            std::condition_variable cvar;
            std::vector<cl_int> errors;

            void addDevice(cl_device_id device_id);
        public:
            explicit Renderer(size_t max_compression_threads);
            ~Renderer();

            Renderer(const Renderer&) = delete;
            Renderer(Renderer&&) = delete;

            Renderer& operator=(const Renderer&) = delete;
            Renderer& operator=(Renderer&&) = delete;

            RenderTask createRenderTask(RenderTaskInfo task_info);

            void runUntilCompletion(RenderTask& task);

            void finishAll();

        private:
            std::pair<size_t, size_t> schedule();

            void launch(RenderTask& task, size_t device_index, size_t frame_index, uint32_t timestamp);

            void targetDownloaded(RenderTask* task, size_t device_index, size_t frame_index, uint32_t timestamp);

            void kernelFailed(RenderTask* task, cl_int status);

            void wait();

            void throwIfKernelError();

            static void targetDownloadedCb(cl_event event, cl_int status, void* user_data);
    };

    struct TargetDownloadedInfo {
        Renderer* renderer;
        RenderTask* task;
        size_t device_index;
        size_t frame_index;
        uint32_t timestamp;
    };
}

#endif
