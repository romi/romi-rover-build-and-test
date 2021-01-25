# Originally from Mark Magath. Modified.
# https://gitlab.com/diplomathesis/i40barman/-/tree/master/product
# Builds the paho.mqtt.c library
#
# This step is necessary to provide the libraries and
# headers for paho.mqtt.cpp project
#
# Add PAHO to root CMakeLiats.txt like so.
#SET(PAHO_BUILD_STATIC TRUE)
#SET(PAHO_BUILD_WITH_SSL FALSE)
#include(${CMAKE_MODULES_DIRECTORY}/pahomqttc.cmake)
#include(${CMAKE_MODULES_DIRECTORY}/pahomqttcpp.cmake)


include(ExternalProject)
set(PAHOMQTTC_DIR ${CMAKE_DOWNLOAD_DIRECTORY}/paho.mqtt.c)
set(PAHOMQTTC_TARGET_DIR ${CMAKE_DOWNLOAD_DIRECTORY}/pahomqttc)
file(MAKE_DIRECTORY ${PAHOMQTTC_TARGET_DIR}/include)

ExternalProject_Add(
    pahomqttc
	PREFIX              ${CMAKE_DOWNLOAD_DIRECTORY}/pahomqttc
	GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git
	GIT_TAG v1.3.8
	 CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PAHOMQTTC_TARGET_DIR}
	 CMAKE_ARGS -DPAHO_WITH_SSL=${PAHO_BUILD_WITH_SSL}
	 CMAKE_ARGS -DPAHO_BUILD_STATIC=${PAHO_BUILD_STATIC}
    BUILD_COMMAND make
	 INSTALL_COMMAND make install
)

# The paho.mqtt.cpp project needs to know where the paho.mqtt.c libraries
# and headers are installed, so we provide this information in an additional
# cmake file which is passed to the paho.mqtt.cpp project build
set(PAHOMQTTC_INCLUDE_DIR ${PAHOMQTTC_TARGET_DIR}/include)
set(PAHOMQTTC_LIBRARY_DIR ${PAHOMQTTC_TARGET_DIR}/lib)

configure_file(${CMAKE_MODULES_DIRECTORY}/fix_pahomqttcpp.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/fix_pahomqttcpp.cmake @ONLY)


add_library(paho-mqttc3::MQTTClient STATIC IMPORTED GLOBAL)
set_target_properties(paho-mqttc3::MQTTClient PROPERTIES
	 IMPORTED_LOCATION ${PAHOMQTTC_TARGET_DIR}/lib/libpaho-mqtt3c.a
	 INTERFACE_INCLUDE_DIRECTORIES ${PAHOMQTTC_TARGET_DIR}/include)
add_library(paho-mqttc3::MQTTAsync STATIC IMPORTED GLOBAL)
set_target_properties(paho-mqttc3::MQTTAsync PROPERTIES
	 IMPORTED_LOCATION ${PAHOMQTTC_TARGET_DIR}/lib/libpaho-mqtt3a.a
	 INTERFACE_INCLUDE_DIRECTORIES ${PAHOMQTTC_TARGET_DIR}/include)

# Example of linkage:
#target_link_libraries(weeder_client_mqtt
#                        PRIVATE paho-mqttpp3
#                        paho-mqttc3::MQTTAsync
#        )