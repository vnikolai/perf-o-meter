# Prepare testing environment

cmake_minimum_required(VERSION 3.14)

include (FetchContent)

FetchContent_Declare (googletest
                      GIT_REPOSITORY https://github.com/google/googletest.git
                      GIT_TAG v1.18.0
)

FetchContent_MakeAvailable (googletest)