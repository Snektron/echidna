#include "client/renderer.hpp"
#include "client/clerror.hpp"
#include "client/clutil.hpp"
#include "utils/log.hpp"

#include <utility>

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
}
