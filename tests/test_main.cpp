#define CATCH_CONFIG_MAIN
#include <iostream>

#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "catch2/catch_session.hpp"
#include "catch2/catch_test_macros.hpp"

constexpr bool hide_stdout = true;

int main(int argc, char* argv[]) {
    absl::InitializeLog();
    absl::SetStderrThreshold(absl::LogSeverity::kWarning);

    if constexpr (hide_stdout) {
        std::cout.setstate(std::ios_base::failbit);  // Hide stdout
    }

    int result = Catch::Session().run(argc, argv);
    return result;
}
