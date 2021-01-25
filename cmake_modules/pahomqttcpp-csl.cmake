#
# Builds the paho.mqtt.cpp library in a simple cmakefile that can easily be imported to other projects.
# Tested with MQTT 1.2.0
# Note: The library is fixed at SHARED for the purposes of the build tests.
# Exports the library: paho-mqttpp3 and the include folders for building locally.
# Example link
#target_link_libraries(weeder_client_mqtt
#		paho-mqttpp3
#		paho-mqtt3a
#		)

# Add the following to the root CMakeLists.txt to use
#include(${CMAKE_MODULES_DIRECTORY}/pahomqttc-csl.cmake)
#include(${CMAKE_MODULES_DIRECTORY}/pahomqttcpp-csl.cmake)

include(${CMAKE_MODULES_DIRECTORY}/DownloadProject.cmake)
download_project(
		PROJ                pahomqttcpp
		GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.cpp.git
		GIT_TAG v1.2.0
		PREFIX              ${CMAKE_DOWNLOAD_DIRECTORY}/pahomqttcpp-dl
		${UPDATE_DISCONNECTED_IF_AVAILABLE}
)

add_subdirectory(${pahomqttcpp_SOURCE_DIR}/src/mqtt)

SET(pahomqttcpp_SRC
		${pahomqttcpp_SOURCE_DIR}/src/async_client.cpp
		${pahomqttcpp_SOURCE_DIR}/src/client.cpp
		${pahomqttcpp_SOURCE_DIR}/src/connect_options.cpp
		${pahomqttcpp_SOURCE_DIR}/src/create_options.cpp
		${pahomqttcpp_SOURCE_DIR}/src/disconnect_options.cpp
		${pahomqttcpp_SOURCE_DIR}/src/iclient_persistence.cpp
		${pahomqttcpp_SOURCE_DIR}/src/message.cpp
		${pahomqttcpp_SOURCE_DIR}/src/properties.cpp
		${pahomqttcpp_SOURCE_DIR}/src/response_options.cpp
		${pahomqttcpp_SOURCE_DIR}/src/ssl_options.cpp
		${pahomqttcpp_SOURCE_DIR}/src/string_collection.cpp
		${pahomqttcpp_SOURCE_DIR}/src/subscribe_options.cpp
		${pahomqttcpp_SOURCE_DIR}/src/token.cpp
		${pahomqttcpp_SOURCE_DIR}/src/topic.cpp
		${pahomqttcpp_SOURCE_DIR}/src/will_options.cpp
		)

add_library(paho-mqttpp3 SHARED ${pahomqttcpp_SRC})


target_include_directories(paho-mqttpp3
		PUBLIC
			"${pahomqttcpp_SOURCE_DIR}/src/mqtt"
			"${pahomqttcpp_SOURCE_DIR}/src"
		)

target_link_libraries(paho-mqttpp3
		paho-mqtt3a
		pthread)