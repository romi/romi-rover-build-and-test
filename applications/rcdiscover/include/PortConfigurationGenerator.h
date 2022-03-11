#ifndef RCDISCOVER_PORTCONFIGURATIONGENERATOR_H
#define RCDISCOVER_PORTCONFIGURATIONGENERATOR_H

#include <string>
#include <vector>
#include "IPortConfigurationGenerator.h"
#include <json.hpp>

class PortConfigurationGenerator : public IPortConfigurationGenerator
{
public:
        PortConfigurationGenerator();
        virtual ~PortConfigurationGenerator() = default;
        
        bool CreateConfigurationFile(const std::string& json_configuration,
                                     const DeviceMap& devices,
                                     const std::string& ouput_file) override;
        [[nodiscard]] std::string LoadConfiguration(const std::string& configuration_file) const override;
        bool SaveConfiguration(const std::string& configuration_file,
                               const std::string& configuration_json) override;
        void CopyNonSerialDevice(const std::string& key, nlohmann::json& value, nlohmann::json& valid_ports);

        
private:
        nlohmann::json CreateConfigurationBase(const std::string& json_configuration);
        nlohmann::json CopyUnhandledDeviceTypes(nlohmann::json& previous_ports_object);
private:
        const std::string serial_ports_configuration_key;
        const std::string serial_port_key;
        const std::string serial_device_type;
        const std::string serial_type;

        PortConfigurationGenerator(const PortConfigurationGenerator& other) = delete;
        PortConfigurationGenerator& operator=(const PortConfigurationGenerator& other) = delete;
};

#endif //RCDISCOVER_PORTCONFIGURATIONGENERATOR_H
