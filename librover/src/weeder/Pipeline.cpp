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

#include "weeder/Pipeline.h"

namespace romi {
        
        void Pipeline::crop_image(ISession& session, Image& camera,
                                  double tool_diameter, Image& crop)
        {
                if (!_cropper.crop(session, camera, tool_diameter, crop)) {
                        throw std::runtime_error("Pipeline: crop failed");
                }
        }

        void Pipeline::create_mask(ISession& session, Image &crop, Image &mask)
        {
                if (!_segmentation.create_mask(session, crop, mask)) {
                        throw std::runtime_error("Pipeline: segmentation failed");
                }
        }

        void Pipeline::trace_path(ISession& session, Image& mask, double tool_diameter,
                                  double meters_to_pixels, Path& path)
        {
                if (!_planner.trace_path(session, mask, tool_diameter,
                                         meters_to_pixels, path)) {
                        throw std::runtime_error("Pipeline: path planner failed");
                }
        }

        void Pipeline::try_run(ISession& session, Image& camera,
                               double tool_diameter, Path& path)
        {
                
                Image crop;
                crop_image(session, camera, tool_diameter, crop);
                session.store_png("crop", crop);

                Image mask;
                create_mask(session, crop, mask);
                session.store_png("mask", mask);

                if (true) {
                        r_debug("Eroding");
                        Image eroded;
                        mask.erode(8, eroded);
                        session.store_png("erode.png", eroded);
                        
                        r_debug("Dilating");
                        eroded.dilate(8, mask);
                        session.store_png("dilate.png", mask);
                }
                

                
                double meters_to_pixels = _cropper.map_meters_to_pixels(1.0);
                // meters_to_pixels = meters_to_pixels / 3.0; // image scaled 1/3
                
                trace_path(session, mask, tool_diameter, meters_to_pixels, path);

                std::vector<size_t> crossings = check_path(session, mask, path);
                if (crossings.size() > 0) {
                        r_warn("Pipeline::try_run: Must reroute path");

                        Image scaled;
                        mask.scale(4, scaled);

                        
                        
                }

                session.store_path("path", 0, path);
                
        }

        bool Pipeline::run(ISession& session, Image& camera,
                           double tool_diameter, Path& path)
        {
                bool success = false;
                try  {
                        try_run(session, camera, tool_diameter, path);
                        success = true;
                        
                } catch (std::runtime_error& e) {
                        r_err("Pipeline::run: try_run failed: %s", e.what());
                }
                
                return success;
        }

        std::vector<size_t> Pipeline::check_path(ISession& session, Image& mask, Path& path)
        {
                rpp::MemBuffer buffer;
                int w = (int) mask.width();
                int h = (int) mask.height();
                
                buffer.printf("<?xml version=\"1.0\" "
                              "encoding=\"UTF-8\" standalone=\"no\"?>"
                              "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" "
                              "xmlns=\"http://www.w3.org/2000/svg\" "
                              "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                              "version=\"1.0\" "
                              "width=\"%dpx\" height=\"%dpx\">\n",
                              w, h);
                
                buffer.printf("    <image xlink:href=\"crop.png\" "
                              "x=\"0\" y=\"0\" "
                              "width=\"%dpx\" height=\"%dpx\" />\n",
                              w, h);
 
                std::vector<size_t> crossings;
                
                for (size_t i = 0; i < path.size() - 1; i++) {
                        if (segment_crosses_plant(buffer, mask, path[i], path[i+1]))
                                crossings.push_back(i);
                }
                
                buffer.printf("</svg>\n");
                session.store_svg("plant-crossings.svg", buffer.tostring());

                return crossings;
        }

        bool Pipeline::segment_crosses_plant(rpp::MemBuffer& buffer,
                                             Image& mask, v3 start, v3 end)
        {
                v3 scale((double) mask.width(), (double) mask.height(), 1.0);
                size_t n = 10;
                size_t count = 0;
                
                for (size_t i = 0; i < n; i++) {
                        v3 p = start + (end - start) * (double) i / (double) n;
                        v3 q = scale * p;
                        float color = mask.get(0, (size_t) q.x(), (size_t) q.y());
                        
                        if (color > 0.0) {
                                count++;
                                buffer.printf("    <circle cx=\"%.1fpx\" cy=\"%.1fpx\" "
                                              "r=\"3px\" fill=\"red\" stroke=\"none\" />\n",
                                              q.x(), q.y());
                        } else {
                                buffer.printf("    <circle cx=\"%.1fpx\" cy=\"%.1fpx\" "
                                              "r=\"3px\" fill=\"blue\" stroke=\"none\" />\n",
                                              q.x(), q.y());
                        }
                }
                return count >= 2;
        }
}

