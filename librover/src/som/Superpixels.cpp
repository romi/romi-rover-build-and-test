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

#include <iostream>
#include "som/Superpixels.h"
#include "cv/slic.h"

namespace romi {

        void convert_32bit_rgb(Image &mask, uint32_t *buf)
        {
                size_t len = mask.width() * mask.height();
                auto data = mask.data();
                for (size_t i = 0; i < len; i++) {
                        float grey_f = data[i];
                        uint32_t grey_i = (uint32_t) (255.0f * grey_f);
                        buf[i] = (grey_i << 16) | (grey_i << 8) | grey_i;
                }
        }

        // PH This vector of pairs is stored X, Y. It should be a struct
        // really to make it obvious.
        std::vector<std::pair<uint32_t, uint32_t>>
        calculate_centres(const std::vector<double> &kseedsl,
                          const std::vector<double> &kseedsa,
                          const std::vector<double> &kseedsb,
                          const std::vector<double> &kseedsx,
                          const std::vector<double> &kseedsy)
        {
                // XY ORDER
                std::vector<std::pair<uint32_t, uint32_t>> centres;
        
                for (size_t xyindex = 0; xyindex < kseedsx.size(); xyindex++) {

                        // PH IMPORTANT: if you uncomment this if statement it
                        // will not plot segments that border a white area.
                        // This is due to colours at a border containing non 0
                        // colour info.
                
                        // if((kseedsl[xyindex] == 0) && (kseedsa[xyindex] == 0) && (kseedsb[xyindex] == 0))
                
                        if ((kseedsl[xyindex] < 1)
                            && (kseedsa[xyindex] < 1)
                            && (kseedsb[xyindex] < 1))
                        
                                centres.push_back(std::pair<uint32_t, uint32_t>((uint32_t)kseedsx[xyindex], (uint32_t)kseedsy[xyindex]));
                }
                return centres;
        }

        std::vector<std::pair<uint32_t, uint32_t>>
        calculate_adjacent_centres(const std::vector<double> &kseedsl, const std::vector<double> &kseedsx, const std::vector<double> &kseedsy)
        {
            // XY ORDER
            std::vector<std::pair<uint32_t, uint32_t>> centres;

            for (size_t xyindex = 0; xyindex < kseedsx.size(); xyindex++)
            {
                if((kseedsl[xyindex] >0 ) && (kseedsl[xyindex] < 1))
                {
                    centres.push_back(std::pair<uint32_t, uint32_t>((uint32_t)kseedsx[xyindex], (uint32_t)kseedsy[xyindex]));
                }

            }
            return centres;
        }

        Centers Superpixels::calculate_centers(Image &mask, int max_centers)
        {
                size_t len = mask.width() * mask.height();
                std::vector<u_int32_t> ubuff(len, 0);
                convert_32bit_rgb(mask, ubuff.data());

                std::vector<int> segmentation_labels;
                size_t segmentation_size = mask.height() * mask.width();
                segmentation_labels.resize(segmentation_size);

                int numlabels(0);

                // This is the number regions * 10 (initial)
                // e.g. segmentation_size/2250 = 225 segments.
                //const int spcount = segmentation_size/2250;

                //   const double compactness = 20.0;
                const double compactness = 80.0;
                std::vector<double> kseedsl(0);
                std::vector<double> kseedsa(0);
                std::vector<double> kseedsb(0);
                std::vector<double> kseedsx(0);
                std::vector<double> kseedsy(0);

                SLIC slic;
                slic.DoSuperpixelSegmentation_ForGivenNumberOfSuperpixels(ubuff.data(),
                                                                          static_cast<int>(mask.width()),
                                                                          static_cast<int>(mask.height()),
                                                                          segmentation_labels, numlabels,
                                                                          max_centers,
                                                                          compactness,
                                                                          kseedsl, kseedsa,
                                                                          kseedsb, kseedsx,
                                                                          kseedsy);
                
                std::cout << "number unique segments (slic) = " << kseedsl.size() << std::endl;

                // Calculate our own centres.
                //   std::vector<uint32_t> unsigned_segmentation_labels(labels, labels + segmentation_size);
                //   auto centres = calculate_centers(unsigned_segmentation_labels, width, height);

                // Use Centres from algo
                Centers centres = calculate_centres(kseedsl, kseedsa, kseedsb, kseedsx, kseedsy);

                return centres;
        }

}
