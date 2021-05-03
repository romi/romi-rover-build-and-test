#ifndef RCDISCOVER_IPORTCONFIGURATIONGENERATOR_H
#define RCDISCOVER_IPORTCONFIGURATIONGENERATOR_H

#include <string>
#include <vector>
#include "ISerialPortIdentification.h"

class IPortConfigurationGenerator {
public:
        IPortConfigurationGenerator() = default;
        virtual ~IPortConfigurationGenerator() = default;
        
        virtual int CreateConfigurationFile(const std::string& json_configuration,
                                            const DeviceMap& devices,
                                            const std::string& ouput_file) = 0;
        virtual std::string LoadConfiguration(const std::string& configuration_file) const = 0;
        virtual bool SaveConfiguration(const std::string& configuration_file,
                                       const std::string& configuration_json) = 0;
};


#endif // RCDISCOVER_IPORTCONFIGURATIONGENERATOR_H
