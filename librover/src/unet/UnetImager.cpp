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

#include <functional>
#include "unet/UnetImager.h"
#include <ClockAccessor.h>

namespace romi {
        
        UnetImager::UnetImager(ISession& session, ICamera& camera)
                : PythonUnet(), Imager(session, camera), grab_queue_(), quit_(false), unet_thread_()
        {
        }
 
        std::string UnetImager::make_output_name()
        {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "mask-%06zu", counter_);
                return std::string(buffer);
        }
 
        std::string UnetImager::get_image_path()
        {
                std::filesystem::path dir = session_.current_path();
                std::filesystem::path path = dir /= make_image_name();
                return path;
        }
 
        void UnetImager::try_unet(std::atomic<bool>& quit)
        {
            while (!quit)
            {
                try {
                    if (grab_queue_.size())
                    {
                        auto imager_params = grab_queue_.pop();
                        if (imager_params.has_value())
                        {
                            send_python_request(imager_params->image_path, imager_params->output_name);
                        }
                    }
                    else
                        rpp::ClockAccessor::GetInstance()->sleep(0.020);
                } catch (const std::runtime_error& e) {
                    r_err("try_unet: exception: %s", e.what());
                }
            }
            r_debug("try_unet: quit");
        }
         
        bool UnetImager::grab()
        {
                bool success = false;
                if (Imager::grab()) {
                        std::string path = get_image_path();
                        std::string name = make_output_name();
                        grab_queue_.push(UnetImagerParams{path, name});
                        success = true;
                }
                return success;
        }

    UnetImager::~UnetImager() {
        stop_unet_processing();
    }

    bool UnetImager::start_recording(const std::string &observation_id, size_t max_images, double max_duration) {
        quit_ = false;
        unet_thread_ = std::make_unique<std::thread>([this]() { try_unet(quit_); });
        return Imager::start_recording(observation_id, max_images, max_duration);
    }

    bool UnetImager::stop_recording() {
        stop_unet_processing();
        return Imager::stop_recording();
    }

    bool UnetImager::is_recording() {
            if (Imager::is_recording() || grab_queue_.size() > 0)
                return true;
            return false;
    }

    void UnetImager::stop_unet_processing() {
        quit_ = true;
        if (unet_thread_) {
            unet_thread_->join();
            unet_thread_ = nullptr;
        }
    }
}
