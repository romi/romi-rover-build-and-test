cmake_minimum_required(VERSION 3.10)
project(romi-rover-build-and-test)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options. Build tests by default. We want to test whether we are developing or building a release.
# I mean... why wouldn't you? :)
option(BUILD_TESTS "Build all tests." ON)

if (BUILD_TESTS)
    set(CMAKE_MODULES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)
    set(CMAKE_DOWNLOAD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/download)

    include(CTest)

    include(${CMAKE_MODULES_DIRECTORY}/CodeCoverage.cmake)

    set( COVERAGE_EXCLUDES
            "/usr/*"
            "*/googletest/*"
            "*/googlemock/*"
            "*test/*")

    append_coverage_compiler_flags()

    enable_testing()

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

endif() # BUILD_TESTS

# Options. Turn on with 'cmake -DCLONE_ROMI_LIB_REPOS=ON'.
# WARNING: Cloning the Romi repos will overwrite any repos you already have. INCLUDING UN-PUSHED CHANGES!
option(CLONE_ROMI_LIB_REPOS "We don't want to clone the romi libs every time we configure cmake." OFF)

if (CLONE_ROMI_LIB_REPOS)
    download_project(
            PROJ              libr
            GIT_REPOSITORY    https://github.com/romi/libr.git
            GIT_TAG           master
            TIMEOUT           180
            SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/libr
            BINARY_DIR        ${CMAKE_BINARY_DIR}/libr
            ${UPDATE_DISCONNECTED_IF_AVAILABLE})
    add_subdirectory(${libr_SOURCE_DIR} ${libr_BINARY_DIR})
    message( "LIBR Binary dir is:" ${libr_BINARY_DIR} )

    download_project(
            PROJ              rcom
            GIT_REPOSITORY    https://github.com/romi/rcom.git
            GIT_TAG           master
            TIMEOUT           180
            SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/rcom
            BINARY_DIR        ${CMAKE_BINARY_DIR}/rcom
            ${UPDATE_DISCONNECTED_IF_AVAILABLE})
    add_subdirectory(${rcom_SOURCE_DIR} ${rcom_BINARY_DIR})
    message( "RCOM Binary dir is:" ${rcom_BINARY_DIR} )

else()
            add_subdirectory(libr)
            add_subdirectory(rcom)
endif()

#add_subdirectory(TestLib)
#add_subdirectory(TestExe)

if (BUILD_TESTS)
    # All tests added at the end due to dependencies.
#    add_subdirectory(TestLib/test)
    add_subdirectory(libr/test)
    add_subdirectory(rcom/test)
endif() # BUILD_TESTS
