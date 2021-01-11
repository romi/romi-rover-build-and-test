/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
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
#ifndef __ROMI_REMOTE_CNC_H
#define __ROMI_REMOTE_CNC_H

#include "rpc/RemoteStub.h"
#include "CNC.h"

namespace romi {

        class RemoteCNC : public CNC, public RemoteStub
        {
        public:
                static constexpr const char *ClassName = "remote-cnc";
                
        public:
                RemoteCNC(rcom::IRPCHandler& rpc_handler)
                        : RemoteStub(rpc_handler) {}
                virtual ~RemoteCNC() override = default;

                bool get_range(CNCRange &range) override;
                bool moveto(double x, double y, double z,
                            double relative_speed = 0.1) override;
                bool spindle(double speed) override;
                bool travel(Path &path, double relative_speed = 0.1) override;
                bool homing() override;

                bool pause_activity() override;
                bool continue_activity() override;
                bool reset_activity() override;
        };
}

#endif // __ROMI_REMOTE_CNC_H
