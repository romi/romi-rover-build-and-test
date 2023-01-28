#include <unistd.h>
#include <stdio.h>
#include <rcom/Linux.h>
#include <hal/I2C.h>
#include <hal/BldcGimbalI2C.h>

int main(int argc, char **argv)
{
        rcom::Linux linux;
        std::unique_ptr<romi::II2C> bus
                = std::make_unique<romi::I2C>(linux,
                                              romi::kGimbalI2CDevice,
                                              11 /*kGimbalI2CAddress*/);
        auto gimbal = std::make_unique<romi::BldcGimbalI2C>(bus);

        printf("Moveto 0\n");
        gimbal->moveto(0.0, 0.0, 0.0, 1.0);

        printf("Sleep 5s\n");
        sleep(5);
        
        printf("Moveto 45°\n");
        gimbal->moveto(45.0, 0.0, 0.0, 1.0);

        printf("Sleep 5s\n");
        sleep(5);

        printf("Done\n");

        return 0;
}
