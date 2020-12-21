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
#include <chrono>
#include <cstdint>

namespace echidna::client {
    // Structure representing device-related task information.
    struct DeviceTaskInfo {
        // Pointer to the device this information is for
        Device* device;

        // Instance of the kernel to render
        UniqueKernel kernel;

        // Vector of FRAMES (See device.hpp) device render targets.
        std::vector<UniqueMemObject> render_targets;

        // Vector of FRAMES (See device.hpp) host render targets. Device render targets
        // are asynchronously copied to this buffer whenever they have finished rendering.
        std::vector<std::vector<uint8_t>> host_render_targets;
    };

    // Structure representing all related information to rendering a batch of frames
    struct RenderTask {
        // The task details
        RenderTaskInfo task_info;

        // Rendering information for each device
        std::vector<DeviceTaskInfo> device_info;

        // The cumulative rendering time of all frames. This is measured on the host
        // over both the GPU and CPU part.
        uint64_t cumulative_frame_time;
    };

    struct TargetDownloadedInfo;

    // Class which represents the renderer.
    class Renderer {
        private:
            // Image format of the device image buffers. This format needs to be compatible with what
            // is passed to the image compression function.
            constexpr static const cl_image_format RENDER_TARGET_FORMAT = {CL_RGBA, CL_UNORM_INT8};

            // A thread pool that is used to schedule image compression jobs to.
            utils::ThreadPool compression_pool;

            // The list of devices this renderer is able to schedule tasks to.
            std::vector<Device> devices;

            // A mutex protecting all information (in RenderTasks) over the main and OpenCL
            // threads.
            std::mutex mutex;

            // A cvar used to yield the main thread while render- and compression work is
            // being performed.
            std::condition_variable cvar;

            // A vector of errors that were returned from OpenCL threads in targetDownloaded.
            std::vector<cl_int> errors;

            // Attempt to add a device to the internal list of devices.
            void addDevice(cl_device_id device_id);
        public:
            // Initialize a new renderer.
            explicit Renderer(size_t max_compression_threads);
            ~Renderer();

            Renderer(const Renderer&) = delete;
            Renderer(Renderer&&) = delete;

            Renderer& operator=(const Renderer&) = delete;
            Renderer& operator=(Renderer&&) = delete;

            // Create a new render task
            RenderTask createRenderTask(RenderTaskInfo task_info);

            // Run a render task until all frames have been rendered, this call is blocking.
            // Returns an average us per frame.
            uint64_t runUntilCompletion(RenderTask& task);

        private:
            // Wait until all OpenCL command queues have finished processing their commands.
            void finishAll();

            // Wait until a [device index, frame index] becomes available for rendering.
            std::pair<size_t, size_t> schedule();

            // Launch a kernel for `task` on device `device_index` and frame `frame_index`.
            void launch(RenderTask& task, size_t device_index, size_t frame_index, uint32_t timestamp);

            // Wait until all compression kernels have performed all their work.
            void wait();

            // OpenCL event callback invoked when an image has been downloaded to host memory.
            static void targetDownloaded(cl_event event, cl_int status, void* user_data);
    };

    // Information passed to the event callback.
    // This contains all relevant information to continue work on `renderer`s associated
    // thread pool and finally signal the renderer that the work has been completed.
    struct TargetDownloadedInfo {
        Renderer* renderer;
        RenderTask* task;
        size_t device_index;
        size_t frame_index;
        uint32_t timestamp;
        std::chrono::steady_clock::time_point start_time;
    };
}

#endif
