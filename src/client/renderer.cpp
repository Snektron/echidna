#include "client/renderer.hpp"
#include "client/clerror.hpp"
#include "client/clutil.hpp"
#include "utils/log.hpp"
#include "utils/string.hpp"

#include <lodepng.h>

#include <utility>
#include <algorithm>

namespace echidna::client {
    void Renderer::addDevice(cl_device_id device_id) {
        // Treat any exception received during device calls as non-fatal, and
        // simply ignore that device.
        auto error_msg = "Skipping device ";
        std::string name;

        try {
            name = "'" + getDeviceName(device_id) + "'";
        } catch (const CLException& err) {
            log::write("Failed to query device name: ", err.what());
            return;
        }

        try {
            if (!isDeviceAvailable(device_id)) {
                log::write(error_msg, name, ": Not available");
                return;
            }

            try {
                auto version = getDeviceVersion(device_id, CL_DEVICE_VERSION);
                if (version < std::make_pair<uint32_t, uint32_t>(1, 2)) {
                    log::write(error_msg, name, ": OpenCL 1.2 not supported");
                    return;
                }
            } catch (const InvalidCLVersionStringException&) {
                log::write(error_msg, name, ": Driver returned invalid version string");
                return;
            }

            this->devices.emplace_back(device_id);
            log::write("Instantiated device ", name);
        } catch (const CLException& err) {
            log::write(error_msg, name, ": Unexpected OpenCL error '", err.what(), "'");
            return;
        }
    }

    Renderer::Renderer(size_t max_compression_threads):
        compression_pool(max_compression_threads) {
        auto device_ids = Device::deviceIDs();

        for (auto device_id : device_ids) {
            this->addDevice(device_id);
        }

        if (this->devices.size() == 0) {
            throw NoDeviceException();
        }

        log::write("Initialized ", this->devices.size(), " out of ", device_ids.size(), " device(s)");
    }

    Renderer::~Renderer() {
        log::write("~Renderer");
        this->finishAll();
    }

    RenderTask Renderer::createRenderTask(RenderTaskInfo task_info) {
        std::vector<DeviceTaskInfo> device_info;
        device_info.reserve(this->devices.size());

        for (auto& device : this->devices) {
            auto kernel = device.buildKernelFromSource(task_info.kernel_source, "render");
            std::vector<UniqueMemObject> render_targets;
            std::vector<std::vector<uint8_t>> host_render_targets;

            for (size_t i = 0; i < device.frames.size(); ++i) {
                render_targets.push_back(
                    device.create2DImage(task_info.image_width, task_info.image_height, RENDER_TARGET_FORMAT)
                );

                host_render_targets.push_back(std::vector<uint8_t>(4 * task_info.image_width * task_info.image_height, 0xAA));
            }

            device_info.push_back({
                &device,
                std::move(kernel),
                std::move(render_targets),
                std::move(host_render_targets),

            });
        }

        return {task_info, std::move(device_info), 0};
    }

    uint64_t Renderer::runUntilCompletion(RenderTask& task) {
        for (auto timestamp : task.task_info.timestamps) {
            auto [device_index, frame_index] = schedule();
            log::write(timestamp, ": launching on device ", device_index, " frame ", frame_index);
            this->launch(task, device_index, frame_index, timestamp);
        }
        this->finishAll();
        this->wait();

        return task.cumulative_frame_time / task.task_info.timestamps.size();
    }

    void Renderer::finishAll() {
        for (auto& device : this->devices) {
            check(clFinish(device.command_queue.get()));
        }
    }

    std::pair<size_t, size_t> Renderer::schedule() {
        while (true) {
            std::unique_lock<std::mutex> lk(this->mutex);
            this->throwIfKernelError();

            for (size_t device_index = 0; device_index < this->devices.size(); ++device_index) {
                auto& device = this->devices[device_index];
                for (size_t frame_index = 0; frame_index < device.frames.size(); ++frame_index) {
                    auto& frame = device.frames[frame_index];

                    if (std::exchange(frame.ready, false)) {
                        return {device_index, frame_index};
                    }
                }
            }

            // No frame is ready, wait
            this->cvar.wait(lk);
        }
    }

    void Renderer::launch(RenderTask& task, size_t device_index, size_t frame_index, uint32_t timestamp) {
        size_t global_work_size[] = {task.task_info.image_width, task.task_info.image_height};

        auto& info = task.device_info[device_index];
        auto* device = info.device;
        auto& render_target = info.render_targets[frame_index];
        auto& host_render_target = info.host_render_targets[frame_index];
        auto& frame = device->frames[frame_index];

        auto start = std::chrono::steady_clock::now();

        check(clSetKernelArg(info.kernel.get(), 0, sizeof(cl_mem), &render_target.get()));
        check(clSetKernelArg(info.kernel.get(), 1, sizeof(cl_uint), &timestamp));

        check(clEnqueueNDRangeKernel(
            device->command_queue.get(),
            info.kernel.get(),
            2,
            nullptr,
            global_work_size,
            nullptr,
            0,
            nullptr,
            &device->frames[frame_index].kernel_completed.get()
        ));

        size_t origin[] = {0, 0, 0};
        size_t region[] = {task.task_info.image_width, task.task_info.image_height, 1};

        check(clEnqueueReadImage(
            device->command_queue.get(),
            render_target.get(),
            CL_FALSE,
            origin,
            region,
            0,
            0,
            host_render_target.data(),
            1,
            &frame.kernel_completed.get(),
            &frame.target_downloaded.get()
        ));

        check(clSetEventCallback(
            frame.target_downloaded.get(),
            CL_COMPLETE,
            &Renderer::targetDownloaded,
            new TargetDownloadedInfo{this, &task, device_index, frame_index, timestamp, start}
        ));
    }

    void Renderer::wait() {
        while (true) {
            std::unique_lock<std::mutex> lk(this->mutex);
            this->throwIfKernelError();

            bool any_unready = false;
            for (size_t device_index = 0; device_index < this->devices.size(); ++device_index) {
                auto& device = this->devices[device_index];
                for (size_t frame_index = 0; frame_index < device.frames.size(); ++frame_index) {
                    auto& frame = device.frames[frame_index];
                    any_unready |= !frame.ready;
                }
            }

            if (!any_unready)
                break;
            this->cvar.wait(lk);
        }
    }

    void Renderer::throwIfKernelError() {
        if (this->errors.size() == 0)
            return;

        throw CLException(this->errors[0]);
    }

    void Renderer::targetDownloaded(cl_event event, cl_int status, void* user_data) {
        // TODO: Check status and throw on main thread
        auto info = std::shared_ptr<TargetDownloadedInfo>(
            reinterpret_cast<TargetDownloadedInfo*>(user_data)
        );

        if (status != CL_COMPLETE) {
            std::unique_lock<std::mutex> lk(info->renderer->mutex);
            info->renderer->errors.push_back(status);
            info->renderer->cvar.notify_one();
            return;
        }

        info->renderer->compression_pool.schedule([info] {
            auto* task = info->task;
            auto& device_info = task->device_info[info->device_index];
            auto* device = device_info.device;
            auto& frame =  device->frames[info->frame_index];

            auto filename = utils::string::make_string(task->task_info.job_id, "-", info->timestamp, ".png");
            lodepng::encode(
                filename,
                device_info.host_render_targets[info->frame_index].data(),
                task->task_info.image_width,
                task->task_info.image_height,
                LCT_RGBA, 8
            );

            auto end_time = std::chrono::steady_clock::now();
            auto frame_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - info->start_time).count();

            log::write(info->timestamp, ": finished in ", frame_time, " us");

            std::lock_guard<std::mutex> lk(info->renderer->mutex);
            info->task->cumulative_frame_time += frame_time;
            frame.ready = true;
            info->renderer->cvar.notify_one();
        });
    }
}
