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

#include "objects/textblitter.h"
#include "include_gl.h"
#include "collision.h"
#include <iostream>
#include "font.h"
#include "render.h"

// TextBlitter

TextBlitter::TextBlitter(int x, int y, int type_id)
: FrameObject(x, y, type_id), flash_interval(0.0f), x_spacing(0), y_spacing(0),
  x_scroll(0), y_scroll(0), anim_type(BLITTER_ANIMATION_NONE),
  charmap_ref(true), callback_line_count(0), draw_image(NULL)
{
    collision = new InstanceBox(this);
}

void TextBlitter::load(const chowstring & filename)
{
    Image * new_image = get_image_cache(filename, 0, 0, 0, 0,
                                        transparent_color);
    if (new_image == NULL) {
        std::cout << "Could not load Text Blitter image " << filename
            << std::endl;
        return;
    }

    image = new_image;
}

TextBlitter::~TextBlitter()
{
    if (!charmap_ref) {
        delete[] charmap;
        delete charmap_str;
    }
    delete collision;
}

void TextBlitter::initialize(const chowstring & map_string)
{
    for (int i = 0; i < 256; i++) {
        charmap[i] = -1;
    }

    for (unsigned int i = 0; i < map_string.size(); i++) {
        unsigned char c = (unsigned char)map_string[i];
        charmap[c] = i;
    }
}

int TextBlitter::get_x_align()
{
    if (alignment & ALIGN_LEFT)
        return 0;
    if (alignment & ALIGN_HCENTER)
        return 1;
    if (alignment & ALIGN_RIGHT)
        return 2;
    return 0;
}

int TextBlitter::get_y_align()
{
    if (alignment & ALIGN_TOP)
        return 0;
    if (alignment & ALIGN_VCENTER)
        return 1;
    if (alignment & ALIGN_BOTTOM)
        return 2;
    return 0;
}

void TextBlitter::set_x_align(int value)
{
    alignment &= ~(ALIGN_LEFT | ALIGN_HCENTER | ALIGN_RIGHT);
    switch (value) {
        case 0:
            alignment |= ALIGN_LEFT;
            break;
        case 1:
            alignment |= ALIGN_HCENTER;
            break;
        case 2:
            alignment |= ALIGN_RIGHT;
            break;
    }
}

void TextBlitter::set_y_align(int value)
{
    alignment &= ~(ALIGN_TOP | ALIGN_VCENTER | ALIGN_BOTTOM);
    switch (value) {
        case 0:
            alignment |= ALIGN_TOP;
            break;
        case 1:
            alignment |= ALIGN_VCENTER;
            break;
        case 2:
            alignment |= ALIGN_BOTTOM;
            break;
    }
}

void TextBlitter::set_x_spacing(int value)
{
    x_spacing = value;
}

void TextBlitter::set_y_spacing(int value)
{
    y_spacing = value;
}

void TextBlitter::set_x_scroll(int value)
{
    x_scroll = value;
}

void TextBlitter::set_y_scroll(int value)
{
    y_scroll = value;
}

void TextBlitter::set_width(int w)
{
    if (w == width)
        return;
    width = w;
    collision->update_aabb();
    update_lines();
}

void TextBlitter::set_height(int h)
{
    height = h;
    collision->update_aabb();
}

void TextBlitter::set_text(const chowstring & value)
{
    if (value == text && !lines.empty())
        return;
    text = value;
    update_lines();
}

void TextBlitter::append_text(const chowstring & value)
{
    text += value;
    update_lines();
}

void TextBlitter::update_lines()
{
    lines.clear();

    if (text.empty()) {
        lines.push_back(LineReference(NULL, 0));
        return;
    }

    int x_add = char_width + x_spacing;

    const char * text_c = text.data();

    bool force_char = false;

    for (unsigned int i = 0; i < text.size(); i++) {
        int start = i;
        int size = 0;
        int last_space = -1;
        bool removed_space = false;

        // find start + end of line
        while (true) {
            if (i >= text.size())
                break;
            unsigned char c = (unsigned char)text_c[i];
            if (c == '\n')
                break;
            if (c == '\r') {
                i++;
                continue;
            }
            if (wrap && ((size + 1) * x_add) > width &&
                    !force_char) {
                if (size == 0)
                    force_char = true;
                if (c == ' ') {
                    size = i - start;
                    break;
                }
                if (last_space != -1) {
                    size = last_space - start;
                    i = last_space;
                }
                i--;
                break;
            }
            force_char = false;
            i++;
            if (c == ' ')
                last_space = i;
            if (c == '\r')
                continue;
            // remove one leading space
            if (wrap && i-1 == start && c == ' ' && lines.size() > 0 &&
                    !removed_space) {
                start++;
                removed_space = true;
                continue;
            }
            size++;
        }

        lines.push_back(LineReference(&text_c[start], size));
    }
}


int TextBlitter::get_line_count()
{
    return int(lines.size());
}

chowstring TextBlitter::get_line(int index)
{
    if (index < 0 || index >= int(lines.size()))
        return empty_string;
    return chowstring(lines[index].start, lines[index].size);
}

chowstring TextBlitter::get_map_char(int i)
{
    if (charmap_str->empty())
        return empty_string;
    i = clamp(i, 0, int(charmap_str->size())-1);
    return charmap_str->substr(i, 1);
}

void TextBlitter::replace_color(Color from, Color to)
{
    replacer.replace(from, to);
}

void TextBlitter::set_transparent_color(int v)
{
    transparent_color = v;
    replacer.set_transparent(transparent_color);
}

void TextBlitter::update()
{
    Image * image;
    if (draw_image == NULL)
        image = this->image;
    else
        image = draw_image;

    if (!replacer.empty())
        draw_image = replacer.apply(image, this->image);

    update_flash(flash_interval, flash_time);

    if (anim_type != BLITTER_ANIMATION_SINWAVE)
        return;
    anim_frame++;
}

void TextBlitter::flash(float value)
{
    flash_interval = value;
    flash_time = 0.0f;
}

void TextBlitter::set_animation_type(int value)
{
    anim_type = value;
}

void TextBlitter::set_animation_speed(int value)
{
    anim_speed = value;
}

void TextBlitter::set_animation_parameter(int index, int value)
{
    switch (index) {
        case 1:
            wave_freq = value;
            break;
        case 2:
            wave_height = value;
            break;
        default:
            std::cout << "Invalid Text Blitter parameter: " << index
                << std::endl;
            break;
    }
}

const chowstring & TextBlitter::get_charmap()
{
    return *charmap_str;
}

void TextBlitter::set_charmap(const chowstring & charmap)
{
    if (charmap_ref) {
        this->charmap = new int[256];
        charmap_ref = false;
    } else {
        delete charmap_str;
    }
    charmap_str = new chowstring(charmap);
    initialize(charmap);
}

void TextBlitter::call_char_callback()
{
}

void TextBlitter::call_line_callback()
{
}

void TextBlitter::call_begin_callback()
{
}

void TextBlitter::draw()
{
    bool has_char_callback = callback_flags & CHAR_CALLBACK;

#ifdef CHOWDREN_BLITTER_EXT_CALLBACKS
    bool has_line_callback = (callback_flags & LINE_CALLBACK) != 0;
    if (callback_flags & BEGIN_CALLBACK)
        call_begin_callback();
#endif

    callback_line_count = int(lines.size());

    Image * image;
    if (draw_image == NULL)
        image = this->image;
    else
        image = draw_image;

    if (!replacer.empty())
        draw_image = image = replacer.apply(image, this->image);

    image->upload_texture();

    begin_draw();

    int x_add = char_width + x_spacing;
    int y_add = char_height + y_spacing;

    int yy = y + y_scroll;
    if (alignment & ALIGN_VCENTER)
        yy += height / 2 - lines.size() * char_height / 2
              - (lines.size()) * y_spacing / 2;

    int bottom_y = y + height;

    int img_width = (image_width / char_width) * char_width;

    int screen_y1 = 0 - (layer->off_y - frame->off_y);
    int screen_y2 = screen_y1 + WINDOW_HEIGHT;

    for (int line_index = 0; line_index < int(lines.size()); ++line_index) {
        if (yy <= y - y_add || yy >= bottom_y ||
            yy + y_add <= screen_y1 || yy >= screen_y2)
        {
            yy += y_add;
            continue;
        }

#ifdef CHOWDREN_BLITTER_EXT_CALLBACKS
        if (has_line_callback)
            call_line_callback();
#endif

        const LineReference & line = lines[line_index];

        int xx = x + x_scroll;

        if (alignment & ALIGN_HCENTER) {
            xx += (width - line.size * x_add) / 2;
        } else if (alignment & ALIGN_RIGHT) {
            xx += width - line.size * x_add;
        }

        // draw line
        for (int i = 0; i < line.size; i++) {
            unsigned char c = (unsigned char)line.start[i];

            int ci = charmap[c - char_offset];
            int img_x = (ci * char_width) % img_width;
            img_x = clamp(img_x + x_off, 0, image->width);
            int img_y = ((ci * char_width) / img_width) * char_height;
            img_y = clamp(img_y + y_off, 0, image->height);

            int x_add_now = x_add;
            int char_width_now = char_width;

            Color color = blend_color;
            int xxx = xx;
            int yyy = yy;
            if (anim_type == BLITTER_ANIMATION_SINWAVE) {
                double t = double(anim_frame / anim_speed + x_add * i);
                t /= double(wave_freq);
                yyy += int(sin(t) * wave_height);
            } else if (has_char_callback) {
                callback_line = line_index;
                callback_char = i;
                callback_transparency = 0;
#ifdef CHOWDREN_BLITTER_EXT_CALLBACKS
                callback_character_char = c;
                callback_char_width = char_width;
                callback_char_src_y = img_y;
                callback_char_dst_x = xxx;
#endif

                call_char_callback();

#ifdef CHOWDREN_BLITTER_EXT_CALLBACKS
                char_width_now = callback_char_width;
                x_add_now -= char_width - char_width_now;
                // xxx = callback_char_dst_x;
                img_y = callback_char_src_y;
#endif

                color.set_semi_transparency(callback_transparency);
            }


            float t_x1 = float(img_x) / float(image->width);
            float t_x2 = float(img_x+char_width_now) / float(image->width);
            float t_y1 = float(img_y) / float(image->height);
            float t_y2 = float(img_y+char_height) / float(image->height);


            Render::draw_tex(xxx, yyy, xxx + char_width_now, yyy + char_height,
                             color, image->tex, t_x1, t_y1, t_x2, t_y2);

            xx += x_add_now;
        }

        yy += y_add;
    }

    end_draw();
}

class DefaultBlitter : public TextBlitter
{
public:
    DefaultBlitter()
    : TextBlitter(0, 0, 0)
    {
        collision = new InstanceBox(this);
        create_alterables();
        this->image = &dummy_image;
        setup_default_instance(this);
    }
};

static DefaultBlitter default_blitter;
FrameObject * default_blitter_instance = &default_blitter;
