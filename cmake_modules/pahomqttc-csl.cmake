#
# Builds the paho.mqtt.c library in a simple cmakefile that can easily be imported to other projects.
# Tested with MQTT 1.3.8
# Note: The library is fixed at SHARED for the purposes of the build tests.

# Exports the libraries: paho-mqtt3c and paho-mqtt3c and the include folder for building locally.
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
		PROJ                pahomqttc
		GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git
		GIT_TAG v1.3.8
		PREFIX              ${CMAKE_DOWNLOAD_DIRECTORY}/pahomqttc-dl
		${UPDATE_DISCONNECTED_IF_AVAILABLE}
		)

CONFIGURE_FILE(${pahomqttc_SOURCE_DIR}/src/VersionInfo.h.in
		${pahomqttc_SOURCE_DIR}/src/VersionInfo.h
		@ONLY
		)
include_directories( "${pahomqttc_SOURCE_DIR}/src/include" )

MESSAGE("MQTTC SRC = ${pahomqttc_SOURCE_DIR}")

		SET(common_src
		${pahomqttc_SOURCE_DIR}/src/MQTTTime.c
		${pahomqttc_SOURCE_DIR}/src/MQTTProtocolClient.c
		${pahomqttc_SOURCE_DIR}/src/Clients.c
		${pahomqttc_SOURCE_DIR}/src/utf-8.c
		${pahomqttc_SOURCE_DIR}/src/MQTTPacket.c
		${pahomqttc_SOURCE_DIR}/src/MQTTPacketOut.c
		${pahomqttc_SOURCE_DIR}/src/Messages.c
		${pahomqttc_SOURCE_DIR}/src/Tree.c
		${pahomqttc_SOURCE_DIR}/src/Socket.c
		${pahomqttc_SOURCE_DIR}/src/Log.c
		${pahomqttc_SOURCE_DIR}/src/MQTTPersistence.c
		${pahomqttc_SOURCE_DIR}/src/Thread.c
		${pahomqttc_SOURCE_DIR}/src/MQTTProtocolOut.c
		${pahomqttc_SOURCE_DIR}/src/MQTTPersistenceDefault.c
		${pahomqttc_SOURCE_DIR}/src/SocketBuffer.c
		${pahomqttc_SOURCE_DIR}/src/LinkedList.c
		${pahomqttc_SOURCE_DIR}/src/MQTTProperties.c
		${pahomqttc_SOURCE_DIR}/src/MQTTReasonCodes.c
		${pahomqttc_SOURCE_DIR}/src/Base64.c
		${pahomqttc_SOURCE_DIR}/src/SHA1.c
		${pahomqttc_SOURCE_DIR}/src/WebSocket.c
		)

ADD_DEFINITIONS(-DHIGH_PERFORMANCE=1)

ADD_LIBRARY(paho-mqtt3c SHARED
		${common_src}
		${pahomqttc_SOURCE_DIR}/src/MQTTClient.c)

ADD_LIBRARY(paho-mqtt3a SHARED
		${common_src}
		${pahomqttc_SOURCE_DIR}/src/MQTTAsync.c
		${pahomqttc_SOURCE_DIR}/src/MQTTAsyncUtils.c)

target_include_directories(paho-mqtt3c
		PUBLIC
			"${pahomqttc_SOURCE_DIR}/src"
		)

target_include_directories(paho-mqtt3a
		PUBLIC
			"${pahomqttc_SOURCE_DIR}/src"
		)