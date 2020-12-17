#include "client/clerror.hpp"

namespace echidna::client {
    std::string_view CLException::errorName(cl_int status) {
        switch (status) {
            case CL_SUCCESS: return "success";
            case CL_DEVICE_NOT_FOUND: return "device not found";
            case CL_DEVICE_NOT_AVAILABLE: return "device not available";
            case CL_COMPILER_NOT_AVAILABLE: return "compiler not available";
            case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "memory object allocation failure";
            case CL_OUT_OF_RESOURCES: return "out of resources";
            case CL_OUT_OF_HOST_MEMORY: return "out of host memory";
            case CL_PROFILING_INFO_NOT_AVAILABLE: return "profiling info not available";
            case CL_MEM_COPY_OVERLAP: return "memory copy overlap";
            case CL_IMAGE_FORMAT_MISMATCH: return "image format mismatch";
            case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "image format not supported";
            case CL_BUILD_PROGRAM_FAILURE: return "program build failure";
            case CL_MAP_FAILURE: return "map failure";
            case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "misaligned sub-buffer offset";
            case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "execution status error for events in wait list";
            case CL_COMPILE_PROGRAM_FAILURE: return "program compilation failure";
            case CL_LINKER_NOT_AVAILABLE: return "linker not available";
            case CL_LINK_PROGRAM_FAILURE: return "program link failure";
            case CL_DEVICE_PARTITION_FAILED: return "device partition failed";
            case CL_KERNEL_ARG_INFO_NOT_AVAILABLE: return "kernel argument info not available";
            case CL_INVALID_VALUE: return "invalid value";
            case CL_INVALID_DEVICE_TYPE: return "invalid device type";
            case CL_INVALID_PLATFORM: return "invalid platform";
            case CL_INVALID_DEVICE: return "invalid device";
            case CL_INVALID_CONTEXT: return "invalid context";
            case CL_INVALID_QUEUE_PROPERTIES: return "invalid queue properties";
            case CL_INVALID_COMMAND_QUEUE: return "invalid command queue";
            case CL_INVALID_HOST_PTR: return "invalid host pointer";
            case CL_INVALID_MEM_OBJECT: return "invalid memory object";
            case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "invalid image format descriptor";
            case CL_INVALID_IMAGE_SIZE: return "invalid image size";
            case CL_INVALID_SAMPLER: return "invalid sampler";
            case CL_INVALID_BINARY: return "invalid binary";
            case CL_INVALID_BUILD_OPTIONS: return "invalid build options";
            case CL_INVALID_PROGRAM: return "invalid program";
            case CL_INVALID_PROGRAM_EXECUTABLE: return "invalid program executable";
            case CL_INVALID_KERNEL_NAME: return "invalid kernel name";
            case CL_INVALID_KERNEL_DEFINITION: return "invalid kernel definition";
            case CL_INVALID_KERNEL: return "invalid kernel";
            case CL_INVALID_ARG_INDEX: return "invalid argument index";
            case CL_INVALID_ARG_VALUE: return "invalid argument value";
            case CL_INVALID_ARG_SIZE: return "invalid argument size";
            case CL_INVALID_KERNEL_ARGS: return "invalid kernel arguments";
            case CL_INVALID_WORK_DIMENSION: return "invalid work dimension";
            case CL_INVALID_WORK_GROUP_SIZE: return "invalid work group size";
            case CL_INVALID_WORK_ITEM_SIZE: return "invalid work item size";
            case CL_INVALID_GLOBAL_OFFSET: return "invalid global offset";
            case CL_INVALID_EVENT_WAIT_LIST: return "invalid event wait list";
            case CL_INVALID_EVENT: return "invalid event";
            case CL_INVALID_OPERATION: return "invalid operation";
            case CL_INVALID_GL_OBJECT: return "invalid OpenGL object";
            case CL_INVALID_BUFFER_SIZE: return "invalid buffer size";
            case CL_INVALID_MIP_LEVEL: return "invalid mip level";
            case CL_INVALID_GLOBAL_WORK_SIZE: return "invalid global work size";
            case CL_INVALID_PROPERTY: return "invalid property";
            case CL_INVALID_IMAGE_DESCRIPTOR: return "invalid image descriptor";
            case CL_INVALID_COMPILER_OPTIONS: return "invalid compiler options";
            case CL_INVALID_LINKER_OPTIONS: return "invalid linker options";
            case CL_INVALID_DEVICE_PARTITION_COUNT: return "invalid device partition count";
            case -1001: return "platform icd not loaded"; // Returned by clGetPlatformIDs when there is no loaded implementation
            default: return "(invalid error)";
        }
    }

    void check(cl_int status) {
        if (status != CL_SUCCESS) {
            throw CLException(status);
        }
    }
}
