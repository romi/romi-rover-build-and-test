#ifndef RCDISCOVER_SERIALPORTIDENTIFICATION_H
#define RCDISCOVER_SERIALPORTIDENTIFICATION_H

#include <iostream>
#include <vector>
#include "ISerialPortDiscover.h"
#include "ISerialPortIdentification.h"

class SerialPortIdentification : ISerialPortIdentification {
public:
        SerialPortIdentification(ISerialPortDiscover& serialPortDiscover);
        virtual ~SerialPortIdentification() = default;
        
        void ConnectedDevices(DeviceList& serialDevices,
                              DeviceMap& devices) override;
private:
        ISerialPortDiscover& serialPortDiscover;
};

#endif //RCDISCOVER_SERIALPORTIDENTIFICATION_H
