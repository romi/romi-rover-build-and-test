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

#include <map>
#include <cv/cv.h>
#include "weeder/Pipeline.h"
#include "astar/AStar.hpp"

namespace romi {

        static const size_t kAstarResolution = 25;
        
        Pipeline::Pipeline(std::unique_ptr<IImageCropper>& cropper,
                           std::unique_ptr<IImageSegmentation>& segmentation,
                           std::unique_ptr<IConnectedComponents>& connected_components,
                           std::unique_ptr<IPathPlanner>& planner)
                : cropper_(),
                  segmentation_(),
                  connected_components_(),
                  planner_()
        {
                cropper_ = std::move(cropper);
                segmentation_ = std::move(segmentation);
                connected_components_ = std::move(connected_components);
                planner_ = std::move(planner);
        }

        std::vector<Path> Pipeline::run(ISession& session, Image& camera,
                                        double tool_diameter)
        {
                std::vector<Path> result;
                try {
                        result = try_run(session, camera, tool_diameter);
                } catch (const std::exception& e) {
                        r_warn("Pipeline::run: caught exception: %s", e.what());
                        r_warn("The path computation failed. Returning an empty path.");
                }
                return result;
        }
        
        std::vector<Path> Pipeline::try_run(ISession& session, Image& camera,
                                            double tool_diameter)
        {       
                Image crop;
                crop_image(session, camera, tool_diameter, crop);
                session.store_png("crop", crop);

                Image mask;
                create_mask(session, crop, mask);
                session.store_png("segmentation", mask);

                romi::filter_mask(mask, mask, 8);
                session.store_png("mask", mask);

                Image dilated_mask;
                
                // TODO
                if (true) {
                        mask.dilate(1 + kAstarResolution / 2, dilated_mask);
                        session.store_png("dilated-mask", dilated_mask);
                } else {
                        dilated_mask = mask;
                }
                
                Image components;
                connected_components_->compute(session, dilated_mask, components);
                session.store_png("components", components);

                double diameter_pixels = cropper_->map_meters_to_pixels(tool_diameter
                                                                        + 0.010);
                size_t max_centers = (size_t) ((double) (mask.width() * mask.height())
                                               / (diameter_pixels * diameter_pixels));

                Centers centers = romi::calculate_centers(dilated_mask, max_centers);

                {
                        rpp::MemBuffer buffer;
                        for (auto & center: centers)
                                buffer.printf("%zu\t%zu\n", center.first, center.second);
                        session.store_txt("centers", buffer.tostring());
                }


                double diameter = cropper_->map_meters_to_pixels(tool_diameter);
                size_t border = (size_t) (diameter / 2.0);
                size_t x0 = border;
                size_t x1 = mask.width() - border;
                size_t y0 = border;
                size_t y1 = mask.height() - border;

                auto it = centers.begin();
                while (it != centers.end()) {

                        size_t cx = (size_t) (*it).first;
                        size_t cy = (size_t) (*it).second;

                        if (cx < x0)
                                cx = x0;
                        if (cx > x1)
                                cx = x1;
                        if (cy < y0)
                                cy = y0;
                        if (cy > y1)
                                cy = y1;

                        float value = dilated_mask.get(Image::kGreyChannel, cx, cy);
                        if (value > 0.0f) {
                                it = centers.erase(it);
                        } else {
                                (*it).first = (uint32_t) cx;
                                (*it).second = (uint32_t) cy;
                                it++;
                        }
                }
                
                std::vector<Centers> component_centers
                        = romi::sort_centers(centers, components);

                r_debug("Pipeline: number of components : %zu", component_centers.size());
                
                char filename[64];
                std::vector<Path> paths;
                paths.push_back(Path());
                
                for (size_t i = 0; i < component_centers.size(); i++) {

                        // Compute shortest path through centers
                        Path initial_path = trace_path(session, component_centers[i], mask);
                        snprintf(filename, sizeof(filename), "path-initial-%02zu", i);
                        session.store_path(filename, 0, initial_path);

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

                                for (size_t k = 0; k < initial_path.size(); k++) {
                                        double x = initial_path[k].x();
                                        double y = initial_path[k].y();
                                        buffer.printf("    <circle cx=\"%dpx\" cy=\"%dpx\" "
                                                      "r=\"3px\" fill=\"red\" stroke=\"none\" />\n",
                                                      (int) x, (int) y);
                                }

                                if (initial_path.size() > 0) {
                                        buffer.printf("    <g>\n");
                                        double x = initial_path[0].x();
                                        double y = initial_path[0].y();
                                        buffer.printf("    <path d=\"M %d,%d",
                                                      (int) x, (int) y);

                                        for (size_t k = 1; k < initial_path.size(); k++) {
                                                x = initial_path[k].x();
                                                y = initial_path[k].y();
                                                buffer.printf(" L %d,%d",
                                                              (int) x, (int) y); 
                                        }
                                        buffer.printf("\" fill=\"none\" "
                                                      "stroke=\"blue\"/>\n");
                                        buffer.printf("    <g>\n");
                                }

                                buffer.printf("</svg>\n");
                
                                char filename[64];
                                snprintf(filename, sizeof(filename), "path-pre-check-%02zu.svg", i);
                                session.store_svg(filename, buffer.tostring());
                        }
                        
                        // check path for plant crossings
                        check_path(session, mask, initial_path, paths, i);
                        
                        // snprintf(filename, sizeof(filename), "path-%02zu", i);
                        // session.store_path(filename, 0, paths.back());
                }

                r_debug("Pipeline: number of paths: %zu", paths.size());
                
                std::vector<Path> normalized_paths;
                for (size_t k = 0; k < paths.size(); k++) {
                                
                        Path path = paths[k]; 
                        Path normalized_path;
                                
                        for (size_t i = 0; i < path.size(); i++) {
                                double x = path[i].x();
                                double y = path[i].y();
                                
                                if (x < (double) x0) {
                                        x = (double) x0;
                                } else if (x > (double) x1) {
                                        x = (double) x1;
                                }
                                
                                if (y < (double) y0) {
                                        y = (double) y0;
                                } else if (y > (double) y1) {
                                        y = (double) y1;
                                }

                                float value = mask.get(Image::kGreyChannel, (size_t) x,
                                                       (size_t) y);
                                if (value > 0.0f) {
                                        r_err("Pipeline::try_run: Failed to re-route path "
                                              "inside workspace without hurting a plant");
                                        throw std::runtime_error("Failed to re-route path");
                                }
                                
                                normalized_path.emplace_back((x - (double) x0) / (double) (x1 - x0),
                                                             (y - (double) y0) / (double) (y1 - y0),
                                                             0.0);
                        }
                        normalized_paths.emplace_back(normalized_path);
                }
                
                return normalized_paths;
        }

        void Pipeline::crop_image(ISession& session, Image& camera,
                                  double tool_diameter, Image& crop)
        {
                if (!cropper_->crop(session, camera, tool_diameter, crop)) {
                        throw std::runtime_error("Pipeline: crop failed");
                }
        }

        void Pipeline::create_mask(ISession& session, Image &crop, Image &mask)
        {
                if (!segmentation_->create_mask(session, crop, mask)) {
                        throw std::runtime_error("Pipeline: segmentation failed");
                }
        }

        Path Pipeline::trace_path(ISession& session, Centers& centers, Image &mask)
        {
                return planner_->trace_path(session, centers, mask);
        }

        void Pipeline::check_path(ISession& session, Image& mask, Path& path,
                                  std::vector<Path>& paths, size_t index)
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

                for (size_t i = 0; i < path.size() - 1; i++) {
                        check_segment(buffer, mask, path[i], path[i+1], paths);
                }
                paths.back().emplace_back(path.back());

                buffer.printf("</svg>\n");
                
                char filename[64];
                snprintf(filename, sizeof(filename), "plant-crossings-%02zu.svg", index);
                session.store_svg(filename, buffer.tostring());
        }
        
        void Pipeline::check_segment(rpp::MemBuffer& buffer,
                                     Image& mask, v3 start, v3 end,
                                     std::vector<Path>& paths)
        {
                if (segment_crosses_white_area(mask, start, end)) {
                        go_around(buffer, mask, start, end, paths);
                } else {
                        //path.emplace_back(start);
                        paths.back().emplace_back(start);
                }
        }

        void Pipeline::go_around(rpp::MemBuffer& buffer, Image& mask, v3 start, v3 end,
                                 std::vector<Path>& paths)
        {
                int d = (int) kAstarResolution;
                int d2 = d / 2;
                int w = (int) mask.width();
                int h = (int) mask.height();
                float *data = mask.data().data();
                float sum;

                AStar::Generator generator;
                generator.setWorldSize({ (int) (w / d), (int) (h / d) });
                generator.setHeuristic(AStar::Heuristic::euclidean);
                generator.setDiagonalMovement(true);

                for (int y = d2; y < h - d2; y += d) {
                        for (int x = d2; x < w - d2; x += d) {
                                int off = y * w + x;

                                // sum the values over an area of size
                                // dxd and centered on (x,y)
                                sum = 0.0f;
                                for (int j = -d2; j <= d2; j++) {
                                        int offy = off + j * w;
                                        for (int i = -d2; i <= d2; i++) {
                                                sum += data[offy + i];
                                        }
                                }
                                
                                if (sum > 0.0f)
                                        generator.addCollision(AStar::Vec2i((int)(x / d),
                                                                            (int)(y / d)));
                        }
                }

                r_debug("Using A* to go around plant, from (%.1f,%.1f) to (%.1f,%.1f)",
                        start.x(), start.y(), end.x(), end.y());
                
                auto new_path = generator.findPath(
                        { (int) (start.x() / (double) d), (int) (start.y() / (double) d) },
                        { (int) (end.x() / (double) d), (int) (end.y() / (double) d) });
                
                int length = (int) new_path.size();


                
                // TODO
                if (length == 0) {
                        r_debug("Pipeline: A*: Failed to find a path");
                        r_debug("From: %f, %f", start.x(), start.y());
                        r_debug("To:   %f, %f", end.x(), end.y());
                        r_debug("Starting new path");
                        paths.back().emplace_back(start);
                        //throw std::runtime_error("*** A*: FAILED TO FIND A PATH ***");
                        
                        // Start a new path
                        paths.push_back(Path());
                        
                } else {
                
                        buffer.printf("    <g>\n");
                        buffer.printf("    <path d=\"M %d,%d L %d,%d\" "
                                      "fill=\"transparent\" stroke=\"blue\"/>\n",
                                      (int) start.x(), (int) start.y(),
                                      (int) end.x(), (int) end.y());
                
                        paths.back().emplace_back(start);
                        //path.emplace_back(start.x(), start.y(), 0.0);
                
                        // The A* algorithm returns the path from end to
                        // start. So we have to iterate backwards. Don't add
                        // the last point. This is the start point... and it
                        // has already been added above. Don't add the first
                        // point (which is the end point) because it will be
                        // added by the next segment (to avoid duplicates).
                
                        for (int i = length-2; i > 0; i--) {
                                int x = (int) d * new_path[(size_t)i].x + d2;
                                int y = (int) d * new_path[(size_t)i].y + d2;
                                buffer.printf("    <circle cx=\"%dpx\" cy=\"%dpx\" "
                                              "r=\"3px\" fill=\"red\" stroke=\"none\" />\n",
                                              x, y);
                                paths.back().emplace_back((double) x, (double) y, 0.0);
                                //path.emplace_back((double) x, (double) y, 0.0);                        
                        }
                        buffer.printf("    </g>\n");
                }
        }
}
