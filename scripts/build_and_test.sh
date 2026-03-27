#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${REPO_ROOT}"

BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="${BUILD_DIR:-cmake_build}"
PARALLEL_JOBS="${PARALLEL_JOBS:-}"

cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
if [[ -n "${PARALLEL_JOBS}" ]]; then
  cmake --build "${BUILD_DIR}" --parallel "${PARALLEL_JOBS}"
else
  cmake --build "${BUILD_DIR}" --parallel
fi
ctest --test-dir "${BUILD_DIR}" --output-on-failure
