
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../include" )

set(SRCS_V
  src/tests_main.cpp
  src/tests_v.cpp
  ../include/v.h
  ../src/v.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_v ${SRCS_V})
target_link_libraries(romi_rover_unit_tests_libromirover_v rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_v
    COMMAND romi_rover_unit_tests_libromirover_v
)



set(SRCS_CNCRANGE
  src/tests_main.cpp
  src/tests_cncrange.cpp
  ../include/CNCRange.h
  ../include/v.h
  ../src/CNCRange.cpp
  ../src/v.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_cncrange ${SRCS_CNCRANGE})
target_link_libraries(romi_rover_unit_tests_libromirover_cncrange rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_cncrange
    COMMAND romi_rover_unit_tests_libromirover_cncrange
)


set(SRCS_PATH
  src/tests_main.cpp
  src/tests_path.cpp
  ../include/CNCRange.h
  ../include/Path.h
  ../include/v.h
  ../src/CNCRange.cpp
  ../src/Path.cpp
  ../src/v.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_path ${SRCS_PATH})
target_link_libraries(romi_rover_unit_tests_libromirover_path rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_path
    COMMAND romi_rover_unit_tests_libromirover_path
)


set(SRCS_IMAGE
  src/tests_main.cpp
  src/tests_image.cpp
  ../include/Image.h
  ../src/Image.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_image ${SRCS_IMAGE})
target_link_libraries(romi_rover_unit_tests_libromirover_image rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_image
    COMMAND romi_rover_unit_tests_libromirover_image
)


set(SRCS_IMAGEIO
  src/tests_main.cpp
  src/tests_imageio.cpp
  ../include/Image.h
  ../include/ImageIO.h
  ../src/Image.cpp
  ../src/ImageIO.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_imageio ${SRCS_IMAGEIO})
target_link_libraries(romi_rover_unit_tests_libromirover_imageio rpp gtest gmock jpeg png)

add_test(
    NAME romi_rover_unit_tests_libromirover_imageio
    COMMAND romi_rover_unit_tests_libromirover_imageio
)


if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_rover_libromirover_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES romi_rover_unit_tests_libromirover_v
                         romi_rover_unit_tests_libromirover_cncrange
                         romi_rover_unit_tests_libromirover_path
                         romi_rover_unit_tests_libromirover_image
                         romi_rover_unit_tests_libromirover_imageio)
endif()

