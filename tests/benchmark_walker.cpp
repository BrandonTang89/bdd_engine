#include <format>

#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"
#include "interp_tester.h"

TEST_CASE("Benchmark Assignments", "[!benchmark]") {
    BENCHMARK("var0 & ... & var4") {
        InterpTester interp;
        constexpr int num_vars = 5;
        for (int i = 0; i < num_vars; i++) {
            interp.feed(std::format("bvar var{};", i));
        }
        interp.feed("set a = true;");
        for (int i = 0; i < num_vars; i++) {
            interp.feed(std::format("set a = var{} & a;", i));
        }
    };

    BENCHMARK("(var0 & ... & var4) | (var5 & ... & var9)") {
        InterpTester interp;
        constexpr int num_vars = 10;
        for (int i = 0; i < num_vars; i++) {
            interp.feed(std::format("bvar var{};", i));
        }

        interp.feed("set a = true;");
        for (int i = 0; i < num_vars / 2; i++) {
            interp.feed(std::format("set a = var{} & a;", i));
        }
        interp.feed("set b = true;");
        for (int i = num_vars / 2; i < num_vars; i++) {
            interp.feed(std::format("set b = var{} & b;", i));
        }
        interp.feed("set c = a | b;");
    };
}
