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

#include "weeder/Weeder.h"

// ToDo: Observation_id
const std::string observation_id = "row_1";


namespace romi {

        Weeder::Weeder(ICamera& camera,
                       IPipeline& pipeline,
                       ICNC& cnc,
                       double z0,
                       double speed,
                       double diameter_tool,
                       ISession& session)
                : _camera(camera),
                  _pipeline(pipeline),
                  _cnc(cnc),
                  _range(),
                  _z0(z0),
                  _speed(speed),
                  _diameter_tool(diameter_tool),
                  session_(session)
        {
                _cnc.get_range(_range);
        }
        
        bool Weeder::hoe()
        {
                r_debug("Weeder::hoe");
                bool success = false;
                
                try {
                        session_.start(observation_id);
                        try_hoe();
                        success = true;
                        
                } catch (std::runtime_error& re) {

                        // Whatever happens, make sure the spindle
                        // stops and the arm goes back up
                        
                        r_debug("Weeder::hoe: catch: %s", re.what());
                        try {
                                stop_spindle_and_move_arm_up();
                        } catch (...) {}
                }
                
                return success;
        }
        
        void Weeder::try_hoe()
        {
                Image image;
                grab_image(image);
                
                std::vector<Path> paths = analyse_image(image);
                
                for (size_t i = 0; i < paths.size(); i++) {
                        adjust_path(paths[i]);
                        store_svg(paths[i], i);
                }

                if (paths.size() > 0) {
                        
                        Path path = paths[0];
                        for (size_t i = 1; i < paths.size(); i++) {
                                append_path(path, paths[i]);
                        }
                        
                        store_svg(path, paths.size());
                        
                        do_hoe(path);
                }
        }

        void Weeder::grab_image(Image& image)
        {
                move_arm_to_camera_position();
                camera_grab(image);
        }

        void Weeder::camera_grab(Image& image)
        {
                if (!_camera.grab(image)) {
                        r_err("Weeder: grab failed");
                        throw std::runtime_error("Weeder: grab failed");
                }
        }

        void Weeder::store_svg(Path& path, size_t index)
        {
                rcom::MemBuffer buffer;
                // The dimensions are in meter. Convert to pixels, width 1000 px/m.
                v3 dimensions = _range.dimensions();
                int w = (int) (dimensions.x() * 1000.0);
                int h = (int) (dimensions.y() * 1000.0);
                buffer.printf("<?xml version=\"1.0\" "
                              "encoding=\"UTF-8\" standalone=\"no\"?>"
                              "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" "
                              "xmlns=\"http://www.w3.org/2000/svg\" "
                              "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                              "version=\"1.0\" "
                              "width=\"%dpx\" height=\"%dpx\">\n",
                              w, h);
                
                buffer.printf("    <image xlink:href=\"workspace.jpg\" "
                              "x=\"0mm\" y=\"0mm\" "
                              "width=\"%dpx\" height=\"%dpx\" />\n",
                              w, h);
                store_svg_path(buffer, path);
                store_svg_centers(buffer, path);
                buffer.printf("</svg>\n");

                char filename[64];
                snprintf(filename, sizeof(filename), "path-%02zu.svg", index);
                
                session_.store_svg(filename, buffer.tostring());
        }

        void Weeder::store_svg_path(rcom::MemBuffer& buffer, Path& path)
        {
                if (path.size() > 1) {
                        buffer.printf("    <path d=\"");

                        v3 dimensions = _range.dimensions();
                        double h = dimensions.y() * 1000.0;
                
                        v3 p = path[0];
                        buffer.printf("M %d,%d L",
                                      (int) (1000.0 * p.x()),
                                      (int) (h - 1000.0 * p.y()));

                        for (size_t index = 1; index < path.size(); index++) {
                                p = path[index];
                                buffer.printf(" %d,%d",
                                              (int) (1000.0 * p.x()),
                                              (int) (h - 1000.0 * p.y()));
                        }

                        buffer.printf("\" id=\"path\" style=\"fill:none;stroke:#0000ce;"
                                      "stroke-width:5;stroke-linecap:butt;"
                                      "stroke-linejoin:miter;stroke-miterlimit:4;"
                                      "stroke-opacity:1;stroke-dasharray:none\" />\n");
                }
        }

        void Weeder::store_svg_centers(rcom::MemBuffer& buffer, Path& path)
        {
                v3 dimensions = _range.dimensions();
                double h = dimensions.y() * 1000.0;
                
                for (size_t index = 0; index < path.size(); index++) {
                        v3 p = path[index];
                        buffer.printf("    <circle cx=\"%dpx\" cy=\"%dpx\" "
                                      "r=\"3px\" fill=\"red\" stroke=\"none\" />\n",
                                      (int) (1000.0 * p.x()),
                                      (int) (h - 1000.0 * p.y()));
                }
        }
        
        std::vector<Path> Weeder::analyse_image(Image& image)
        {
                return _pipeline.run(session_, image, _diameter_tool);
        }
        
        void Weeder::do_hoe(Path &path)
        {
                r_debug("Weeder::do_hoe");
                move_arm_to_start_position(path[0]);
                travel(path, _speed);
                stop_spindle_and_move_arm_up();
                move_arm_to_camera_position();
        }
        
        void Weeder::move_arm_to_camera_position()
        {
                stop_spindle_and_move_arm_up();
                moveto(0.0, _range.max_.y(), 0.0);
        }

        void Weeder::move_arm_to_start_position(v3 p)
        {
                r_debug("Weeder::move_arm_to_start_position");

                stop_spindle_and_move_arm_up();
                moveto(p.x(), p.y(), 0.0);
                start_spindle();
                moveto(p.x(), p.y(), _z0);
        }

        void Weeder::stop_spindle_and_move_arm_up()
        {
                r_debug("Weeder::stop_spindle_and_move_arm_up");
                stop_spindle();
                moveto(ICNC::UNCHANGED, ICNC::UNCHANGED, 0.0);
        }
        
        void Weeder::adjust_path(Path& path)
        {
                r_debug("Weeder::adjust_path");
                scale_to_range(path);

                // TODO: TSP paths aren't necessary closed loops!
                //rotate_path_to_starting_point(path);
                
                // for (size_t i = 0; i < out.size(); i++) {
                //         r_debug("Point: %.3f, %.3f, %.3f",
                //                 out[i].x(), out[i].y(), out[i].z());
                // }
        }
        
        void Weeder::append_path(Path& combined, Path& extension)
        {
                if (extension.size() > 0) {
                        v3 from = combined.back();
                        v3 to = extension.front();

                        // Move to next path at z=0, i.e. with arm lifted.
                        combined.emplace_back(v3(from.x(), from.y(), 0.0));
                        combined.emplace_back(v3(to.x(), to.y(), 0.0));
                        combined.insert(combined.end(), extension.begin(), extension.end());
                }
        }
        
        void Weeder::scale_to_range(Path& path)
        {
                r_debug("Weeder::scale_to_range");

                // The y-axis of the rover is inverted with respect to
                // the image coordinates.
                path.invert_y();

                // The path has x,y coordinates in the range [0,1].
                // Map these coordinates to the physical dimensions of
                // the workspace.
                path.scale(_range.dimensions());
                path.translate(_range.min_);
                path.set_z(_z0);
                
                if (!path.clamp(_range, 0.01))
                        throw std::runtime_error("Computed path out of range");
        }
        
        void Weeder::rotate_path_to_starting_point(Path& path)
        {
                r_debug("Weeder::rotate_path_to_starting_point");
                v3 starting_point(0.0, _range.max_.y(), _z0);
                int closest_index = path.closest_point(starting_point);
                if (closest_index >= 0) {
                        Path out;
                        path.rotate(out, (size_t) closest_index);
                        path = out;
                } else {
                        throw std::runtime_error("rotate_path_to_starting_point empty path, no closest point");
                }
        }
        
        void Weeder::travel(Path& path, double v)
        {
                if (!_cnc.travel(path, v)) {
                        r_err("Weeder: travel failed");
                        throw std::runtime_error("Weeder: travel failed");
                }
        }

        void Weeder::moveto(double x, double y, double z)
        {
                if (!_cnc.moveto(x, y, z, _speed)) {
                        r_err("Weeder: moveto failed");
                        throw std::runtime_error("Weeder: moveto failed");
                }
        }

        void Weeder::start_spindle()
        {
                if (!_cnc.spindle(1.0)) {
                        r_err("Weeder: start spindle failed");
                        throw std::runtime_error("Weeder: start spindle failed");
                }
        }
        
        void Weeder::stop_spindle()
        {
                if (!_cnc.spindle(0.0)) {
                        r_err("Weeder: stop spindle failed");
                        throw std::runtime_error("Weeder: stop spindle failed");
                }
        }

        bool Weeder::stop()
        {
                return _cnc.spindle(0.0);
        }
        
        bool Weeder::pause_activity()
        {
                return _cnc.pause_activity();
        }
        
        bool Weeder::continue_activity()
        {
                return _cnc.continue_activity();
        }
        
        bool Weeder::reset_activity()
        {
                return _cnc.reset_activity();
        }

        bool Weeder::power_up()
        {
                return _cnc.power_up();
        }
        
        bool Weeder::power_down()
        {
                return _cnc.power_down();
        }
        
        bool Weeder::stand_by()
        {
                return _cnc.stand_by();
        }
        
        bool Weeder::wake_up()
        {
                return _cnc.wake_up();
        }
}
