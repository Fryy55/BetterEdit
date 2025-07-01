#pragma once

#include <Geode/utils/general.hpp>

using namespace geode::prelude;

namespace pro::ssl {
    bool verify(std::string const& message, ByteVector const& messageSignature);
}
