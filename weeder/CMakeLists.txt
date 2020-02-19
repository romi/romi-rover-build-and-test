cmake_minimum_required(VERSION 2.6)
project(weeder)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(weeder weeder.c weeder_main.c weeding.c svm.c otsu.c quincunx.c profiling.c workspace.c point.c)
target_link_libraries(weeder rcom romi r m)

add_executable(weeder_compute weeder_compute.c weeding.c svm.c otsu.c quincunx.c profiling.c workspace.c point.c)
target_link_libraries(weeder_compute romi r m)

add_executable(weeder_eval weeder_eval.c weeding.c svm.c otsu.c quincunx.c profiling.c workspace.c point.c)
target_link_libraries(weeder_eval romi r m)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/weeder_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/weeder_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )

install(TARGETS weeder weeder_eval weeder_compute DESTINATION romi)
