# Prepare testing environment

cmake_minimum_required(VERSION 3.14)

include (FetchContent)

FetchContent_Declare (googlebenchmark
                      GIT_REPOSITORY https://github.com/google/benchmark.git
                      GIT_TAG v1.9.5
)

FetchContent_MakeAvailable (googlebenchmark)