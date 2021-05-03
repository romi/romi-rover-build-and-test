// Thanks to:
// Geoffrey Hunter <gbmhunter@gmail.com> (www.mbedded.ninja)
// For serial virtual port setup.
#include <iostream>
#include <gtest/gtest.h>

#include "TestUtil.hpp"

using namespace CppLinuxSerial;

class Environment : public testing::Environment {
public:
    virtual ~Environment() = default;
    // Override this to define how to set up the environment.
    virtual void SetUp() {
        std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
        TestUtil::GetInstance().CreateVirtualSerialPortPair();
    }
    // Override this to define how to tear down the environment.
    virtual void TearDown() {
        TestUtil::GetInstance().CloseSerialPorts();
    }
};

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    // Create and register global test setup
    // (gtest takes ownership of pointer, do not delete manaully!)
    ::testing::AddGlobalTestEnvironment(new Environment);
    return RUN_ALL_TESTS();
}