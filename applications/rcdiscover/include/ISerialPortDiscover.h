#ifndef RCDISCOVER_ISERIALPORT_DISCOVER_H
#define RCDISCOVER_ISERIALPORT_DISCOVER_H

class ISerialPortDiscover {
public:
        virtual ~ISerialPortDiscover() = default;
        virtual std::string ConnectedDevice(const std::string& path) = 0;
};
#endif //RCDISCOVER_ISERIALPORT_DISCOVER_H
