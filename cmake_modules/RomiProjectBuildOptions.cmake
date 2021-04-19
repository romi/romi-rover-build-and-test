MESSAGE("Importing Romi Project Build Settings" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# We override this on the server for a test buid, so tests should be run with this on before checking in.
option(ADDRESS_SANITISER_BUILD "Enable build settings to allow address sanitiser" ON) # default to ON

if(ADDRESS_SANITISER_BUILD)
    message(WARNING "Enabling GCC address sanitiser don't use valgrind with sanitizer enabled.")
    set( PROJECT_SANITISE_FLAGS
            "-fsanitize=address,undefined,shift,integer-divide-by-zero,unreachable,vla-bound,null,return,signed-integer-overflow,bounds,alignment,object-size,float-divide-by-zero,float-cast-overflow,nonnull-attribute,returns-nonnull-attribute,bool,enum,vptr -fno-omit-frame-pointer")
else()
    MESSAGE("libr sanitize flags off")
    set( PROJECT_SANITISE_FLAGS "")
endif()

# For now everything is debug. Set Compile options locally to maintain independent library builds.
# Ideal list of warnings! Add gradually!
#-Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs -Wuseless-cast -Wconversion -Wsign-conversion -Wswitch-default -Weffc++ -Wswitch-enum -Wzero-as-null-pointer-constant -Winit-self -Wformat=2 -Waddress -Wlogical-op -Wpointer-arith)
set(COMMON_COMPILATION_FLAGS "-Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wswitch-default -Wswitch-enum -Winit-self -Waddress -Wlogical-op -Wpointer-arith -Wformat=2")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${COMMON_COMPILATION_FLAGS} -Weffc++ -Wzero-as-null-pointer-constant ${PROJECT_SANITISE_FLAGS}")
set(CMAKE_C_FLAGS "${COMMON_COMPILATION_FLAGS} ${PROJECT_SANITISE_FLAGS}")

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PROJECT_SANITISE_FLAGS}")

# Place this after the include so that the flags are appended and not overwritten.
if (BUILD_COVERAGE AND BUILD_TESTS)
    append_coverage_compiler_flags()
endif()
############################################################

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

############################################################