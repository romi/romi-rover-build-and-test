#ifndef _CABLEBOTMOTORCONTROLLER_ISTATE_H
#define _CABLEBOTMOTORCONTROLLER_ISTATE_H

enum DeviceState {
        STATE_ERROR,
        STATE_STARTING_UP,
        STATE_INITIALIZING_DEBUG_SERIAL,
        STATE_INITIALIZING_CAMERA_SERIAL,
        STATE_INITIALIZING_MOTORCONTROLLER,
        STATE_INITIALIZING_POSITION,
        STATE_INITIALIZING_SWITCHES,
        STATE_READY
};

enum DeviceError {
        ERROR_NONE = 0,
        ERROR_ODRIVE,
        ERROR_BATTERY,
        ERROR_HOMING
};

enum BatteryState {
        STATE_BATTERY_OK,
        STATE_BATTERY_LOW,
        STATE_BATTERY_CRITICAL
};

class IState
{
public:
        virtual ~IState() = default;

        virtual void init() = 0;
        virtual void set(DeviceState s) = 0;
        virtual void set_error(DeviceError error, const char *message) = 0;
        virtual DeviceError error() const = 0;
        virtual const char *message() const = 0;
        virtual void set_battery(float voltage) = 0;
        virtual void clear_error() = 0;
        virtual void update() = 0;
};

#endif // _CABLEBOTMOTORCONTROLLER_ISTATE_H


