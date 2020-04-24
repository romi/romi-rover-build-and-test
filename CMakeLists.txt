cmake_minimum_required(VERSION 3.10)
project(romi-rover-build-and-test)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

############################################################

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Options.
#Build tests by default. We want run tests whether we are developing or building a release. I mean... why wouldn't you? :)
option(BUILD_TESTS "Build all tests." ON)
option(BUILD_COVERAGE "Build coverage." ON)

# Build the developer example applications. These cannot be built with Coverage enabled.
option(BUILD_EXAMPLES "Build example applications." OFF)

############################################################

# Test early on if all the complimentary options are enabled. Exit if flags aren't correct.
# This avoids downloading libs and supporting tools when the confifure won't work later on.
if (BUILD_EXAMPLES)
    if (BUILD_COVERAGE)
        Message(FATAL_ERROR "Turn Coverage off with -DBUILD_COVERAGE=OFF to build examples.")
    endif()
endif()

############################################################

if(BUILD_TESTS)
    set(CMAKE_MODULES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)
    set(CMAKE_DOWNLOAD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)

    include(CTest)
    enable_testing()

    if (NOT DEFINED googletest_SOURCE_DIR)
        if (CMAKE_VERSION VERSION_LESS 3.2)
            set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
        else()
            set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
        endif()

        include(${CMAKE_MODULES_DIRECTORY}/DownloadProject.cmake)
        download_project(   PROJ                googletest
                GIT_REPOSITORY      https://github.com/google/googletest.git
                GIT_TAG             master
                PREFIX              ${CMAKE_DOWNLOAD_DIRECTORY}/googletest
                ${UPDATE_DISCONNECTED_IF_AVAILABLE}
                )

        # Prevent GoogleTest from overriding our compiler/linker options
        # when building with Visual Studio
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    endif()

endif() # BUILD_TESTS

# Can't build for coverage without the test.
if (BUILD_COVERAGE AND BUILD_TESTS)
    if (NOT DEFINED lcov_SOURCE_DIR)
        # lcov doesn't build with CMake so we download, set lcov as an external target, then call make.
        # we then manually add the bin path so that we can call it later.
        download_project(   PROJ              lcov
                URL               https://github.com/linux-test-project/lcov/releases/download/v1.14/lcov-1.14.tar.gz
                PREFIX            ${CMAKE_DOWNLOAD_DIRECTORY}/lcov
                TIMEOUT           180
                ${UPDATE_DISCONNECTED_IF_AVAILABLE}
                )

        set(LCOV_PATH "${lcov_SOURCE_DIR}/bin/lcov")
        set(GENHTML_PATH "${lcov_SOURCE_DIR}/bin/genhtml")
    endif()

    include(${CMAKE_MODULES_DIRECTORY}/CodeCoverage.cmake)

    append_coverage_compiler_flags()
    set( COVERAGE_EXCLUDES
            "/usr/*"
            "*/googletest/*"
            "*/googlemock/*"
            "*test/*")
endif()

######################################

if (NOT DEFINED libr_SOURCE_DIR)
    download_project(
            PROJ              libr
            GIT_REPOSITORY    https://github.com/romi/libr.git
            GIT_TAG           ci_dev
            TIMEOUT           180
            SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/libr
#            BINARY_DIR        ${CMAKE_BINARY_DIR}/libr
            ${UPDATE_DISCONNECTED_IF_AVAILABLE})
    # For coverage we want to add the test to the local build.
    set(BUILD_TESTS  OFF)
    add_subdirectory(${libr_SOURCE_DIR} ${libr_BINARY_DIR})
    set(BUILD_TESTS  ON)
endif()

###########################################################3

if (NOT DEFINED rcom_SOURCE_DIR)
    download_project(
            PROJ              rcom
            GIT_REPOSITORY    https://github.com/romi/rcom.git
            GIT_TAG           ci_dev
            TIMEOUT           180
            SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/rcom
#            BINARY_DIR        ${CMAKE_BINARY_DIR}/rcom
            ${UPDATE_DISCONNECTED_IF_AVAILABLE})
    add_subdirectory(${rcom_SOURCE_DIR} ${rcom_BINARY_DIR})
endif()

###########################################################3

# We've checked above for the correct build options, but another check of the coverage flag won't hurt.
if (BUILD_EXAMPLES)
    if (NOT BUILD_COVERAGE)
        add_subdirectory(rcom/examples)
    endif()
endif()