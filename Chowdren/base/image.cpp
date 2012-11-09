#include <string>
#include "image.h"
#include "stb_image.c"
#include "string.h"
#include "color.h"
#include <iostream>
#include "assets.h"

Image::Image(int handle, int hot_x, int hot_y, int act_x, int act_y) 
: handle(handle), hotspot_x(hot_x), hotspot_y(hot_y), action_x(act_x), 
action_y(act_y), tex(NULL), image(NULL)
{
}

Image::~Image()
{
    if (image != NULL)
        stbi_image_free(image);
}

Image::Image(const std::string & filename, int hot_x, int hot_y, 
             int act_x, int act_y, Color * color) 
: hotspot_x(hot_x), hotspot_y(hot_y), action_x(act_x), action_y(act_y),
  tex(NULL), image(NULL)
{
    load_filename(filename, color);
}

void Image::load()
{
    if (image != NULL)
        return;
    load_filename(
        get_assets_folder() + "/" + number_to_string(handle) + ".png");
}

void Image::load_filename(const std::string & filename, Color * color)
{
    if (image != NULL)
        return;

    int channels;
    image = stbi_load(filename.c_str(), &width, &height, &channels, 4);

    if(image == NULL) {
        printf("Could not load %s\n", filename.c_str());
        return;
    }

    if ((channels == 1 || channels == 3) && color != NULL) {
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                unsigned char * data = &(image[(y * width + x)*4]);
                if (data[0] == color->r && data[1] == color->g && 
                    data[2] == color->b)
                    data[3] = 0;
                else
                    data[3] = 255;
            }
        }
    }
}

void Image::upload_texture()
{
    if (tex != NULL)
        return;
    else if (image == NULL) {
        tex = NULL;
        return;
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, 
                 GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

unsigned int & Image::get(int x, int y)
{
    return ((unsigned int*)image)[y * width + x];
}

void Image::draw(double x, double y, double angle, 
                 double scale_x, double scale_y,
                 bool flip_x, bool flip_y, GLuint background)
{
    load();
    upload_texture();

    if (tex == NULL)
        return;

    glPushMatrix();
    glTranslated(x, y, 0.0);
    glRotated(-angle, 0.0, 0.0, 1.0);
    glScaled(scale_x, scale_y, 1.0);
    x -= (double)hotspot_x;
    y -= (double)hotspot_y;
    if (background != NULL)
        glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    if (background != NULL) {
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, background);
    }
    glBegin(GL_QUADS);
    float tex_coords[8];
    if (flip_x) {
         tex_coords[0] = 1.0; tex_coords[1] = 0.0;
         tex_coords[2] = 0.0; tex_coords[3] = 0.0;
         tex_coords[4] = 0.0; tex_coords[5] = 1.0;
         tex_coords[6] = 1.0; tex_coords[7] = 1.0;
    } else {
         tex_coords[0] = 0.0; tex_coords[1] = 0.0;
         tex_coords[2] = 1.0; tex_coords[3] = 0.0;
         tex_coords[4] = 1.0; tex_coords[5] = 1.0;
         tex_coords[6] = 0.0; tex_coords[7] = 1.0;
    }
    glTexCoord2f(tex_coords[0], tex_coords[1]);
    if (background != NULL)
        glMultiTexCoord2f(GL_TEXTURE1, 0.0, 1.0);
    glVertex2d(-hotspot_x, -hotspot_y);
    glTexCoord2f(tex_coords[2], tex_coords[3]);
    if (background != NULL)
        glMultiTexCoord2f(GL_TEXTURE1, 1.0, 1.0);
    glVertex2d(-hotspot_x + width, -hotspot_y);
    glTexCoord2f(tex_coords[4], tex_coords[5]);
    if (background != NULL)
        glMultiTexCoord2f(GL_TEXTURE1, 1.0, 0.0);
    glVertex2d(-hotspot_x + width, -hotspot_y + height);
    glTexCoord2f(tex_coords[6], tex_coords[7]);
    if (background != NULL)
        glMultiTexCoord2f(GL_TEXTURE1, 0.0, 0.0);
    glVertex2d(-hotspot_x, -hotspot_y + height);
    glEnd();
    if (background != NULL) {
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
    }
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}