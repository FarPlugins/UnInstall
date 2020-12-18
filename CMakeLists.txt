cmake_minimum_required(VERSION 3.15)
project(UnInstall CXX)

#====================================================
# Enable policy
#====================================================
# enable CMAKE_MSVC_RUNTIME_LIBRARY
cmake_policy(SET CMP0091 NEW)

#====================================================
# Set default build to release
#====================================================
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type, one of: Release, Debug" FORCE)
endif ()
message("Build type for UnInstall: ${CMAKE_BUILD_TYPE}")

# check platform for build
set(IS_x64_PLATFORM FALSE)
if ("$ENV{PLATFORM}" MATCHES "x64")
  set(IS_x64_PLATFORM TRUE)
endif ()

add_subdirectory(./src)