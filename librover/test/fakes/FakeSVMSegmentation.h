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

#ifndef __ROMI_FAKE_SVM_SEGMENTATION_H
#define __ROMI_FAKE_SVM_SEGMENTATION_H

#include "session/ISession.h"
#include "weeder/IImageSegmentation.h"

namespace romi {

        class FakeSVMSegmentation : public IImageSegmentation
        {
        public:
                explicit FakeSVMSegmentation(Image& mask);

                virtual ~FakeSVMSegmentation() override = default;
                bool create_mask(ISession &session, Image &image, Image &mask) override;
        private:
            romi::Image mask_;
        };
}

#endif // __ROMI_SVM_SEGMENTATION_H
