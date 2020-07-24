/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include "StateTransition.h"
#include "StateMachine.h"
#include "IStateMachineTimer.h"
#include "IArduino.h"

#ifndef __STATE_MACHINE_CONTROL_PANEL_H
#define __STATE_MACHINE_CONTROL_PANEL_H

enum {
        // Both relays off.
        STATE_OFF = 1,
        // Power relay off and controller relay on. Waiting for
        // controller circuit to send powerup event.
        STATE_STARTING_UP,
        // Power relay off and controller relay remains on for
        // 5s. This gives the controller the time to shut down
        // cleanly.
        STATE_SHUTTING_DOWN,
        // Both relays on
        STATE_ON             
};

enum {
        EVENT_READY,
        EVENT_SELECT_HELD,
        EVENT_POWERUP,
        EVENT_SHUTDOWN,
        EVENT_POWERDOWN
};

class Ready : public StateTransition
{
public:
        Ready() : StateTransition(IStateMachine::StartState,
                                  EVENT_READY,
                                  STATE_OFF) {}
        
        virtual int doTransition() { return 0; }
};

class StartUp : public StateTransition
{
protected:
        IArduino *_arduino;
        
public:
        StartUp(IArduino *arduino) : StateTransition(STATE_OFF,
                                                     EVENT_SELECT_HELD,
                                                     STATE_STARTING_UP),
                                     _arduino(arduino) {}
        
        virtual int doTransition();
};

class PowerUp : public StateTransition
{
protected:
        IArduino *_arduino;
        
public:
        // The power down event is sent by the control software when
        // the controllers are initialized and ready to go.
        PowerUp(IArduino *arduino) : StateTransition(STATE_STARTING_UP,
                                                     EVENT_POWERUP,
                                                     STATE_ON),
                                     _arduino(arduino) {}
        
        virtual int doTransition();
};

class Shutdown : public StateTransition
{
protected:
        IArduino *_arduino;
        IStateMachineTimer *_timer;
        
public:
        // The shutdown event is sent by the control software when the
        // user requested to turn of the robot trough the user
        // interface. It powers off the motors and waits 5s before
        // shutting down the controllers. This gives the controller
        // the time to shut down cleanly.
        Shutdown(IArduino *arduino, IStateMachineTimer *timer)
                : StateTransition(STATE_ON,
                                  EVENT_SHUTDOWN,
                                  STATE_SHUTTING_DOWN),
                  _arduino(arduino),
                  _timer(timer) {}
        
        virtual int doTransition();
};

class SoftPowerDown : public StateTransition
{
protected:
        IArduino *_arduino;
        
public:
        // The soft power down event is sent 5s after the shutdown
        // event.
        SoftPowerDown(IArduino *arduino) : StateTransition(STATE_SHUTTING_DOWN,
                                                           EVENT_POWERDOWN,
                                                           STATE_OFF),
                                           _arduino(arduino) {}
        
        virtual int doTransition();
};

class HardPowerDown : public StateTransition
{
protected:
        IArduino *_arduino;
        
public:
        // The hard power down event is sent when the user presses and
        // holds the select button. It powers off the motors and the
        // controllers.
        HardPowerDown(IArduino *arduino, int from) : StateTransition(from,
                                                                     EVENT_SELECT_HELD,
                                                                     STATE_OFF),
                                                     _arduino(arduino) {}
        
        virtual int doTransition();
};

#endif // __STATE_MACHINE_CONTROL_PANEL_H
