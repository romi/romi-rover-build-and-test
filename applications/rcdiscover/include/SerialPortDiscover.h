#ifndef RCDISCOVER_SERIALPORTDISCOVER_H
#define RCDISCOVER_SERIALPORTDISCOVER_H

#include <map>
#include <RSerial.h>
#include "ISerialPortDiscover.h"

class SerialPortDiscover : public ISerialPortDiscover {
public:
        SerialPortDiscover(const std::map<std::string, std::string>& deviceFilter);
        virtual ~SerialPortDiscover() = default;
        std::string ConnectedDevice(const std::string& path) override;

private:
        std::string TryConnectedDevice(const std::string& path);
        std::string FilterDevice(std::string& info);
private:
        const std::map<std::string, std::string>& knownDevicesMap;
};

#endif //RCDISCOVER_SERIALPORTDISCOVER_H
