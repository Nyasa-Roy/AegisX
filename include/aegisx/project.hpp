#pragma once

#include <string_view>

namespace aegisx {

[[nodiscard]] constexpr std::string_view project_name() noexcept {
    return "AegisX";
}

}  // namespace aegisx
