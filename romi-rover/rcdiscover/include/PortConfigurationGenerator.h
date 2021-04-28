#ifndef RCDISCOVER_PORTCONFIGURATIONGENERATOR_H
#define RCDISCOVER_PORTCONFIGURATIONGENERATOR_H

#include <string>
#include <vector>
#include "IPortConfigurationGenerator.h"
#include <json.h>

class PortConfigurationGenerator : public IPortConfigurationGenerator
{
public:
        PortConfigurationGenerator();
        virtual ~PortConfigurationGenerator() = default;
        
        int CreateConfigurationFile(const std::string& json_configuration,
                                    const DeviceMap& devices,
                                    const std::string& ouput_file) override;
        std::string LoadConfiguration(const std::string& configuration_file) const override;
        bool SaveConfiguration(const std::string& configuration_file,
                               const std::string& configuration_json) override;
        void CopyDevicePerhaps(const char* key, json_object_t value);
        
private:
        json_object_t CreateConfigurationBase(const std::string& json_configuration);
        void CopyUnhandledDeviceTypes(json_object_t previous_ports_object);
private:
        const std::string serial_ports_configuration_key;
        const std::string serial_port_key;
        const std::string serial_device_type;
        const std::string serial_type;
        json_object_t ports_;

        PortConfigurationGenerator(const PortConfigurationGenerator& other) = delete;
        PortConfigurationGenerator& operator=(const PortConfigurationGenerator& other) = delete;
};

#endif //RCDISCOVER_PORTCONFIGURATIONGENERATOR_H
