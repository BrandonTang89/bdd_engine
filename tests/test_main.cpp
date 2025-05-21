#define CATCH_CONFIG_MAIN
#include <iostream>

#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "catch2/catch_session.hpp"

constexpr bool hide_stdout = true;

int main(const int argc, char* argv[]) {
    absl::InitializeLog();
    absl::SetStderrThreshold(absl::LogSeverity::kWarning);

    if constexpr (hide_stdout) {
        std::cout.setstate(std::ios_base::failbit);  // Hide stdout
    }

    const int result = Catch::Session().run(argc, argv);
    return result;
}
