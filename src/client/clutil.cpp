#include "client/clutil.hpp"
#include <sstream>

namespace echidna::client {
    std::string getDeviceName(cl_device_id device_id) {
        size_t size;
        check(clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, nullptr, &size));
        std::string name(size, 0);
        check(clGetDeviceInfo(device_id, CL_DEVICE_NAME, size, name.data(), nullptr));
        return name;
    }

    bool isDeviceAvailable(cl_device_id device_id) {
        cl_bool available;
        check(clGetDeviceInfo(device_id, CL_DEVICE_AVAILABLE, sizeof available, &available, nullptr));
        return available;
    }

    std::pair<uint32_t, uint32_t> getDeviceVersion(cl_device_id device_id, cl_device_info version_info) {
        size_t size;
        check(clGetDeviceInfo(device_id, version_info, 0, nullptr, &size));
        std::string version_str(size, 0);
        check(clGetDeviceInfo(device_id, version_info, size, version_str.data(), nullptr));
        return parseVersion(version_str);
    }

    cl_device_type getDeviceType(cl_device_id device_id) {
        cl_device_type type;
        check(clGetDeviceInfo(device_id, CL_DEVICE_TYPE, sizeof type, &type, nullptr));
        return type;
    }

    cl_int getEventExecutionStatus(cl_event event) {
        cl_int event_status;
        check(clGetEventInfo(event, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof event_status, &event_status, nullptr));
        return event_status;
    }

    std::pair<uint32_t, uint32_t> parseVersion(std::string_view version_str) {
        auto opencl_prefix = std::string_view("OpenCL ");
        if (version_str.rfind(opencl_prefix) == 0)
            version_str.remove_prefix(opencl_prefix.size());

        auto c_prefix = std::string_view("C ");
        if (version_str.rfind(c_prefix) == 0)
            version_str.remove_prefix(c_prefix.size());

        uint32_t major, minor;

        auto ss = std::stringstream();
        ss << version_str;
        ss >> major;
        if (ss.fail() || ss.get() != '.')
            throw InvalidCLVersionStringException();

        ss >> minor;
        if (ss.fail())
            throw InvalidCLVersionStringException();

        return std::make_pair(major, minor);
    }
}
