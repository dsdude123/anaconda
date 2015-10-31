// Copyright (c) Mathias Kaerlev 2012-2015.
//
// This file is part of Anaconda.
//
// Anaconda is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Anaconda is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Anaconda.  If not, see <http://www.gnu.org/licenses/>.

#include "objects/alphaimage.h"

enum {
    ALPHA_SCALE = 1,
    ALPHA_RESIZE = 2
};

AlphaImageObject::AlphaImageObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), anim_frame(0), index(0), image(NULL),
  scale_x(1.0), scale_y(1.0), new_width(-1), new_height(-1), alpha_flags(0)
{
}

void AlphaImageObject::set_image(int index)
{
    this->index = index;
    image = &images[index];
    width = image->width;
    height = image->height;
}

void AlphaImageObject::set_hotspot(int x, int y)
{
    hotspot_x = x;
    hotspot_y = y;
}

void AlphaImageObject::set_scale_x(double scale_x)
{
    this->scale_x = scale_x;
    alpha_flags &= ~ALPHA_RESIZE;
    alpha_flags |= ALPHA_SCALE;
    new_width = new_height = -1;
}

void AlphaImageObject::set_scale_y(double scale_y)
{
    this->scale_y = scale_y;
    alpha_flags &= ~ALPHA_RESIZE;
    alpha_flags |= ALPHA_SCALE;
    new_width = new_height = -1;
}

void AlphaImageObject::set_width(int value)
{
    new_width = value;
    if (new_height == -1)
        new_height = image->height;
    alpha_flags &= ~ALPHA_SCALE;
    alpha_flags |= ALPHA_RESIZE;
    scale_x = scale_y = 1.0;
}

void AlphaImageObject::set_height(int value)
{
    new_height = value;
    if (new_width == -1)
        new_width = image->width;
    alpha_flags &= ~ALPHA_SCALE;
    alpha_flags |= ALPHA_RESIZE;
    scale_x = scale_y = 1.0;
}

int AlphaImageObject::get_width()
{
    if (alpha_flags & ALPHA_SCALE) {
        return image->width * scale_x;
    } else if (alpha_flags & ALPHA_RESIZE) {
        return new_width;
    }
    return image->width;
}

int AlphaImageObject::get_height()
{
    if (alpha_flags & ALPHA_SCALE) {
        return image->height * scale_y;
    } else if (alpha_flags & ALPHA_RESIZE) {
        return new_height;
    }
    return image->height;
}
