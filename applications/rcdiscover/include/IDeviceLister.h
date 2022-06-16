#ifndef RCDISCOVER_IDEVICELISTER_H
#define RCDISCOVER_IDEVICELISTER_H

#include <vector>
#include <utility>
#include <string>

using DeviceList = std::vector<std::string>;
using DeviceMap = std::vector<std::pair<std::string, std::string>>;

class IDeviceLister {
public:
        virtual ~IDeviceLister() = default;
        virtual void ListFilesOfType(const std::string& directory,
                                     const std::string& type,
                                     DeviceList& devices) = 0;
};

#endif // RCDISCOVER_IDEVICELISTER_H
