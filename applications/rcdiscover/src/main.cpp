#include <map>
#include "PortConfigurationGenerator.h"
#include "SerialPortDiscover.h"
#include "SerialPortIdentification.h"
#include "DeviceLister.h"

std::string dev = "/dev";

/* Since the names that controllers return don't match the JSON name in config files,
 * we need to map the device info that is returned when a device is queried to json
 * key in the config file. */
const std::map<std::string, std::string> serial_device_to_json_key = {
        { "Grbl", "grbl" },
        { "Alarm", "grbl" },
        { "Idle", "grbl" },
        { "BrushMotorController", "brush-motor-driver" },
        { "SabertoothController", "brush-motor-driver" },
        { "CrystalDisplay", "display-device" },
        { "Oquam", "oquam" },
        { "ToolCarrierJrkG2", "tool_carrier" }
};

// LCOV_EXCL_START

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " <optional> config_file.json\n"
              << "\tIf an existing config file is passed as a parameter\n"
              << "\tany discovered ports will be added.\n"
              << "\tIf no file is passed a new file (serial_port_cfg.json) is created.\n"
              << std::endl;
}

int main(int argc, char* argv[])
{
    std::string port_configuration_file("serial_port_cfg.json");
    std::string json_configuration;
    DeviceLister deviceLister;
    SerialPortDiscover serialPortDiscover(serial_device_to_json_key);
    SerialPortIdentification serialPortIdentification(serialPortDiscover);
    PortConfigurationGenerator portConfigurationGenerator;

    if (argc > 2) {
        show_usage(argv[0]);
        return 1;
    } else if (argc == 2) {
        port_configuration_file = argv[1];
        json_configuration = portConfigurationGenerator.LoadConfiguration(port_configuration_file);
    }

    DeviceList serialDevices;
    deviceLister.ListFilesOfType(dev, std::string("ttyACM"), serialDevices);
    deviceLister.ListFilesOfType(dev, std::string("ttyUSB"), serialDevices);
    
    DeviceMap connectedDevices;
    serialPortIdentification.ConnectedDevices(serialDevices, connectedDevices);
    
    auto configuration_result = portConfigurationGenerator.CreateConfigurationFile(json_configuration,
                                                                                   connectedDevices,
                                                                                   port_configuration_file);
    if (configuration_result == 0) {
        std::cout << "wrote config: " << port_configuration_file << std::endl;
    } else {
        std::cout << "failed to write config: " << port_configuration_file << std::endl;
    }

    return 0;
}

// LCOV_EXCL_END
