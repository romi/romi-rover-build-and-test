
#ifndef ROMI_ROVER_BUILD_AND_TEST_SERIALPORTDISCOVER_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_SERIALPORTDISCOVER_MOCK_H

#include "gmock/gmock.h"
#include "ISerialPortDiscover.h"

namespace Mocks
{
#pragma GCC diagnostic push
// Google mocks macro constructor initialisation
#pragma GCC diagnostic ignored "-Weffc++"
    class SerialPortDiscoverMock : public ISerialPortDiscover
    {
    public:
        MOCK_METHOD1(ConnectedDevice, std::string (const std::string& path));
    };
#pragma GCC diagnostic pop
}

#endif //ROMI_ROVER_BUILD_AND_TEST_SERIALPORTDISCOVER_MOCK_H
