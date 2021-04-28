#ifndef RCDISCOVER_DEVICELISTER_H
#define RCDISCOVER_DEVICELISTER_H

#include "IDeviceLister.h"

class DeviceLister : IDeviceLister {
public:
        DeviceLister();
        virtual ~DeviceLister() = default;
        
        void ListFilesOfType(const std::string& directory,
                             const std::string& type,
                             DeviceList& devices) override;

protected:
        void TryListFilesOfType(const std::string& directory,
                                const std::string& type,
                                std::vector<std::string>& devices);
};

#endif //RCDISCOVER_DEVICELISTER_H
