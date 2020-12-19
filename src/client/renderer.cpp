#include "client/renderer.hpp"
#include "client/clerror.hpp"
#include "client/clutil.hpp"
#include "utils/log.hpp"

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

    Renderer::Renderer() {
        auto device_ids = Device::deviceIDs();

        for (auto device_id : device_ids) {
            this->addDevice(device_id);
        }

        if (this->devices.size() == 0) {
            throw NoDeviceException();
        }

        log::write("Initialized ", this->devices.size(), " out of ", device_ids.size(), " device(s)");

        // Make sure GPU devices will get priority
        std::partition(this->devices.begin(), this->devices.end(), [](const Device& device) {
            return device.type_mask & CL_DEVICE_TYPE_GPU;
        });
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

            for (size_t i = 0; i < device.events.size(); ++i) {
                render_targets.push_back(
                    device.create2DImage(task_info.image_width, task_info.image_height, RENDER_TARGET_FORMAT)
                );
            }

            device_info.push_back({
                &device,
                std::move(kernel),
                std::move(render_targets),
            });
        }

        return {task_info, std::move(device_info)};
    }

    void Renderer::runUntilCompletion(const RenderTask& task) {
        size_t global_work_size[] = {task.task_info.image_width, task.task_info.image_height};

        auto launch = [&](const DeviceTaskInfo& device_info, size_t image_index) {
            auto* device = device_info.device;
            auto& image = device_info.render_targets[image_index];

            check(clSetKernelArg(device_info.kernel, 0, sizeof(cl_mem), &image));
            check(clEnqueueNDRangeKernel(
                device->command_queue,
                device_info.kernel,
                2,
                nullptr,
                global_work_size,
                nullptr,
                0,
                nullptr,
                &static_cast<cl_event&>(device->events[image_index])
            ));
        };

        auto trySchedule = [&]() -> std::pair<size_t, size_t> {
            while (true) {
                for (size_t j = 0; j < task.device_info.size(); ++j) {
                    auto& info = task.device_info[j];
                    for (size_t i = 0; i < info.render_targets.size(); ++i) {
                        cl_int event_status = getEventExecutionStatus(info.device->events[i]);
                        switch (event_status) {
                            case CL_COMPLETE:
                                return {j, i};
                            case CL_QUEUED:
                            case CL_SUBMITTED:
                            case CL_RUNNING:
                                continue;
                            default:
                                // TODO: This returns an error when the kernel has a problem
                                // and should be handled more appropriately
                                check(event_status);
                        }
                    }
                }
            }
        };

        for (auto timestamp : task.task_info.timestamps) {
            auto [device_index, image_index] = trySchedule();
            log::write(timestamp, ": launching on device ", device_index, " image ", image_index);
            launch(task.device_info[device_index], image_index);
        }
    }

    void Renderer::finishAll() {
        for (auto& device : this->devices) {
            check(clFinish(device.command_queue));
        }
    }
}
