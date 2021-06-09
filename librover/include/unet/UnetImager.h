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

#ifndef __ROMI_UNET_IMAGER_H
#define __ROMI_UNET_IMAGER_H
#include <atomic>
#include <ThreadsafeQueue.h>
#include <camera/Imager.h>
#include "unet/PythonUnet.h"

class UnetImagerParams
{
public:
    std::string image_path;
    std::string output_name;
};

namespace romi {

        class UnetImager : public PythonUnet, public Imager
        {
        private:
            ThreadsafeQueue<UnetImagerParams> grab_queue_;
            std::atomic<bool> quit_;
            std::unique_ptr<std::thread> unet_thread_;
            void stop_unet_processing();
        protected:
                
                bool grab() override;
                std::string make_output_name();
                std::string get_image_path();
                void try_unet(std::atomic<bool>& quit);

        public:
                UnetImager(ISession& session, ICamera& camera);
                ~UnetImager() override;
            bool start_recording(const std::string& observation_id,
                                 size_t max_images,
                                 double max_duration) override;
            bool stop_recording() override;
            bool is_recording() override;
        };
}

#endif // __ROMI_UNET_IMAGER_H
