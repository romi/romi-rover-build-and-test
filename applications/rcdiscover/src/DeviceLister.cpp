#include "DeviceLister.h"

#include <iostream>
#include <stdexcept>
#include <filesystem>
namespace fs = std::filesystem;

DeviceLister::DeviceLister()
{
}

void DeviceLister::ListFilesOfType(const std::string& directory,
                                               const std::string& type,
                                               std::vector<std::string>& devices)
{
        try {
                TryListFilesOfType(directory, type, devices);
        } catch (std::exception& e) {
                std::cout << e.what() << std::endl;
        }
}

void DeviceLister::TryListFilesOfType(const std::string& directory,
                                      const std::string& type,
                                      std::vector<std::string>& devices)
{
        if (type.length() > 0) {
                for (auto& p : fs::directory_iterator(directory)) {
                        if (p.path().string().find(type) != std::string::npos)
                                devices.emplace_back(p.path());
                }
        }
}
