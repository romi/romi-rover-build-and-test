#include <iostream>
#include <vector>
#include <fstream>

#include "PortConfigurationGenerator.h"
#include "json.hpp"


PortConfigurationGenerator::PortConfigurationGenerator()
        : serial_ports_configuration_key("ports"),
          serial_port_key("port"),
          serial_device_type("type"),
          serial_type("serial")
{
}

nlohmann::json
PortConfigurationGenerator::CreateConfigurationBase(const std::string& json_configuration)
{
    nlohmann::json configuration_object;
    try {
        configuration_object = nlohmann::json::parse(json_configuration);
    }
    catch(nlohmann::json::exception& ex){
        configuration_object =  nlohmann::json::object();
    }
    return configuration_object;

}

void PortConfigurationGenerator::CopyNonSerialDevice(const std::string& key, nlohmann::json& value,
                                                     nlohmann::json& valid_ports)
{
        std::string type = value["type"];
        if (type != serial_type) {
                std::cout << "Copying " << key << std::endl;
                valid_ports[key] = value;
        }
}

nlohmann::json
PortConfigurationGenerator::CopyUnhandledDeviceTypes(nlohmann::json& previous_ports_object)
{
    nlohmann::json valid_ports{};
    for (auto& el : previous_ports_object.items())
    {
        CopyNonSerialDevice(el.key(), el.value(), valid_ports);
    }
    return valid_ports;
}

bool
PortConfigurationGenerator::CreateConfigurationFile(const std::string& json_configuration,
                                                    const DeviceMap& devices,
                                                    const std::string& ouput_file)
{

        nlohmann::json configuration_object = CreateConfigurationBase(json_configuration);
        nlohmann::json ports{};
        nlohmann::json valid_ports = nlohmann::json::object();;
        if (configuration_object.contains(serial_ports_configuration_key))
        {
            ports = configuration_object["serial_ports_configuration_key"];
            valid_ports = CopyUnhandledDeviceTypes(ports);
        }

        for (const auto& device : devices) {
            nlohmann::json  device_object{};
            device_object[serial_device_type] = serial_type;
            device_object[serial_port_key] = device.first;
            valid_ports[device.second] = device_object;
        }
        configuration_object[serial_ports_configuration_key] = valid_ports;
        return SaveConfiguration(ouput_file, configuration_object.dump(4));
}

std::string
PortConfigurationGenerator::LoadConfiguration(const std::string& configuration_file) const
{
        std::ostringstream contents;
        try {
                std::ifstream in;
                in.exceptions(std::ifstream::badbit | std::ifstream::failbit);
                in.open(configuration_file);
                contents << in.rdbuf();
        } catch (const std::exception& ex) {
                std::cout << __PRETTY_FUNCTION__  << " failed to load file: '"
                          << configuration_file << "' exception: " << ex.what()
                          << std::endl;
        }
        return (contents.str());
}

bool PortConfigurationGenerator::SaveConfiguration(const std::string& configuration_file,
                                                   const std::string& configuration_json)
{
        bool result = true;
        try {
                std::ofstream out;
                out.exceptions(std::ifstream::badbit | std::ifstream::failbit);
                out.open(configuration_file);
                out << configuration_json;
                std::cout << "wrote config: " << configuration_file << std::endl;
        } catch (const std::exception& ex) {
                std::cout << __PRETTY_FUNCTION__  << " failed to write file: '"
                          << configuration_file << "' exception: " << ex.what()
                          << std::endl;
                result = false;
        }
        return result;
}
