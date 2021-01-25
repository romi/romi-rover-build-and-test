# Originally from Mark Magath. Modified.
# https://gitlab.com/diplomathesis/i40barman/-/tree/master/product
# Builds the paho.mqtt.cpp library
#
# Outputs the following target:
#   paho-mqttpp3
#
include(ExternalProject)
set(PAHOMQTTCPP_DIR ${CMAKE_DOWNLOAD_DIRECTORY}/paho.mqtt.cpp)
set(PAHOMQTTCPP_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR}/paho-build)
set(PAHOMQTTCPP_STATIC_LIB ${PAHOMQTTCPP_TARGET_DIR}/lib/libpaho-mqttpp3.a) 

file(MAKE_DIRECTORY ${PAHOMQTTCPP_TARGET_DIR}/include)

set(PAHO_BUILD_SHARED FALSE)
if(NOT ${PAHO_BUILD_STATIC})
  set(PAHO_BUILD_SHARED TRUE)
endif()

ExternalProject_Add(
    pahomqttcpp
	 DEPENDS pahomqttc
	PREFIX              ${CMAKE_DOWNLOAD_DIRECTORY}/pahomqttcpp
	GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.cpp.git
	GIT_TAG v1.2.0
	 CMAKE_ARGS -DCMAKE_PROJECT_paho-mqtt-cpp_INCLUDE=${CMAKE_CURRENT_BINARY_DIR}/fix_pahomqttcpp.cmake
	 CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PAHOMQTTCPP_TARGET_DIR}
	 CMAKE_ARGS -DPAHO_WITH_SSL=${PAHO_BUILD_WITH_SSL} 
	 CMAKE_ARGS -DPAHO_BUILD_STATIC=${PAHO_BUILD_STATIC} 
	 CMAKE_ARGS -DPAHO_BUILD_SHARED=${PAHO_BUILD_SHARED}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
	 BUILD_BYPRODUCTS ${PAHOMQTTCPP_STATIC_LIB}
)

add_library(paho-mqttpp3 STATIC IMPORTED GLOBAL)

add_dependencies(paho-mqttpp3 pahomqttcpp)

set_target_properties(paho-mqttpp3 PROPERTIES IMPORTED_LOCATION ${PAHOMQTTCPP_STATIC_LIB})
set_target_properties(paho-mqttpp3 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${PAHOMQTTCPP_TARGET_DIR}/include)

# Example of linkage:
#target_link_libraries(weeder_client_mqtt
#                        PRIVATE paho-mqttpp3
#                        paho-mqttc3::MQTTAsync
#        )