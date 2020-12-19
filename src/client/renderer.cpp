#include "client/renderer.hpp"
#include "client/clerror.hpp"
#include "client/clutil.hpp"
#include "utils/log.hpp"

#include <utility>
#include <algorithm>
#include <iostream>

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

    Renderer::Renderer() {
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
        this->finishAll();
    }

    RenderTask Renderer::createRenderTask(RenderTaskInfo task_info) {
        std::vector<DeviceTaskInfo> device_info;
        device_info.reserve(this->devices.size());

        for (auto& device : this->devices) {
            auto kernel = device.buildKernelFromSource(task_info.kernel_source, "render");
            std::vector<UniqueMemObject> render_targets;
            std::vector<std::vector<uint32_t>> host_render_targets;

            for (size_t i = 0; i < device.frames.size(); ++i) {
                render_targets.push_back(
                    device.create2DImage(task_info.image_width, task_info.image_height, RENDER_TARGET_FORMAT)
                );

                host_render_targets.push_back(std::vector<uint32_t>(task_info.image_width * task_info.image_height));
            }

            device_info.push_back({
                &device,
                std::move(kernel),
                std::move(render_targets),
                std::move(host_render_targets),
            });
        }

        return {task_info, std::move(device_info)};
    }

    void Renderer::runUntilCompletion(RenderTask& task) {
        for (auto timestamp : task.task_info.timestamps) {
            auto [device_index, frame_index] = schedule(task);
            log::write(timestamp, ": launching on device ", device_index, " image ", frame_index);
            this->launch(task, device_index, frame_index);
        }
        this->finishAll();
    }

    void Renderer::finishAll() {
        for (auto& device : this->devices) {
            check(clFinish(device.command_queue.get()));
        }
    }

    std::pair<size_t, size_t> Renderer::schedule(RenderTask& task) {
        while (true) {
            std::unique_lock<std::mutex> lk(this->mutex);
            for (size_t device_index = 0; device_index < task.device_info.size(); ++device_index) {
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

    void Renderer::launch(RenderTask& task, size_t device_index, size_t frame_index) {
        size_t global_work_size[] = {task.task_info.image_width, task.task_info.image_height};

        auto& info = task.device_info[device_index];
        auto* device = info.device;
        auto& render_target = info.render_targets[frame_index];
        auto& host_render_target = info.host_render_targets[frame_index];
        auto& frame = device->frames[frame_index];

        check(clSetKernelArg(info.kernel.get(), 0, sizeof(cl_mem), &render_target.get()));
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
            &Renderer::targetDownloadedCb,
            new TargetDownloadedInfo{this, &task, device_index, frame_index}
        ));
    }

    void Renderer::targetDownloaded(RenderTask& task, size_t device_index, size_t frame_index) {
        auto& info = task.device_info[device_index];
        auto* device = info.device;
        auto& frame = device->frames[frame_index];

        std::lock_guard<std::mutex> lk(this->mutex);
        frame.ready = true;
        this->cvar.notify_one();
    }

    void Renderer::targetDownloadedCb(cl_event event, cl_int status, void* user_data) {
        auto info = std::unique_ptr<TargetDownloadedInfo>(
            reinterpret_cast<TargetDownloadedInfo*>(user_data)
        );

        info->renderer->targetDownloaded(*info->task, info->device_index, info->frame_index);
    }
}
