#ifndef RCDISCOVER_ISERIALPORTIDENTIFICATION_H
#define RCDISCOVER_ISERIALPORTIDENTIFICATION_H

#include <vector>
#include <utility>
#include <string>
#include "IDeviceLister.h"

using DeviceMap = std::vector<std::pair<std::string, std::string>>;

class ISerialPortIdentification {
public:
        virtual ~ISerialPortIdentification() = default;
        
        virtual void ConnectedDevices(DeviceList& serialDevices,
                                      DeviceMap& devices) = 0;
};

#endif //RCDISCOVER_ISERIALPORTIDENTIFICATION_H
