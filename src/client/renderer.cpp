#include "client/renderer.hpp"
#include "client/clerror.hpp"

#include <utility>

namespace echidna::client {
    Renderer::Renderer(std::vector<Device>&& devices):
        devices(std::move(devices)) {
    }
}
