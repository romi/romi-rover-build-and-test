############################################################

function(output_compile_flags)
    MESSAGE("${PROJECT_NAME} Build Type = ${CMAKE_BUILD_TYPE}")
    MESSAGE("${PROJECT_NAME} CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
    MESSAGE("${PROJECT_NAME} CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
    MESSAGE("${PROJECT_NAME} CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE} CMAKE_C_FLAGS_RELEASE = ${CMAKE_C_FLAGS_RELEASE}")
    MESSAGE("${PROJECT_NAME} CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG} CMAKE_C_FLAGS_DEBUG = ${CMAKE_C_FLAGS_DEBUG}")
endfunction()

############################################################
function(set_romi_build_defaults PI_BUILD)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv6l")
        set(${PI_BUILD} TRUE PARENT_SCOPE)
        MESSAGE("PI ZERO PROCESSOR DETECTED ${CMAKE_SYSTEM_PROCESSOR} PI_BUILD = ${PI_BUILD}")
   else()
        set(${PI_BUILD} FALSE PARENT_SCOPE)
        MESSAGE("NON PI ZERO PROCESSOR DETECTED ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
    endif()
endfunction()

############################################################
