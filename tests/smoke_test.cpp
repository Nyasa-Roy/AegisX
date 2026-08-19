#include <string_view>

#include <gtest/gtest.h>

#include "aegisx/project.hpp"

TEST(ProjectFoundation, ExposesProjectName) {
    EXPECT_EQ(aegisx::project_name(), std::string_view{"AegisX"});
}
