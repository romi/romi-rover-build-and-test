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
    if((CMAKE_SYSTEM_PROCESSOR MATCHES ".*arm") OR (CMAKE_SYSTEM_PROCESSOR MATCHES ".*aarch64"))
        set(${PI_BUILD} TRUE PARENT_SCOPE)
        MESSAGE("ARM PROCESSOR DETECTED ${CMAKE_SYSTEM_PROCESSOR} PI_BUILD = ${PI_BUILD}")
    else()
        MESSAGE("X86 PROCESSOR DETECTED ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
    endif()
endfunction()

############################################################
