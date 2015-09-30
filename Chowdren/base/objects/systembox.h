#ifndef CHOWDREN_SYSTEMBOX_H
#define CHOWDREN_SYSTEMBOX_H

#include "frameobject.h"
#include "color.h"
#include "font.h"
#include <string>

#ifdef CHOWDREN_USE_GWEN
#include "Gwen/Controls/Button.h"
#endif

class SystemBox : public FrameObject
{
public:
    FRAMEOBJECT_HEAD(SystemBox)

    enum DrawType
    {
        PATTERN_IMAGE,
        CENTER_IMAGE,
        TOPLEFT_IMAGE
    };

    enum BoxFlags
    {
        CHECKED = 1 << 7
    };

    Image * image;
    int type;
    std::string text;
    FTSimpleLayout * layout;
    Color box_color;
    int box_flags;

#ifdef CHOWDREN_USE_GWEN
    Gwen::Controls::Button * button;
    int margin[4];
    int clicked;
    void update();
    void init_button();
#endif

    SystemBox(int x, int y, int type_id);
    ~SystemBox();
    void set_text(const std::string & text);
    void set_size(int w, int h);
    void draw();
    void hide_border_1();
    void hide_border_2();
    void hide_fill();
    void set_border_1(Color color);
    void set_border_2(Color color);
    void set_fill(Color color);
    void check();
    void uncheck();
    void disable();
    const std::string & get_font_name();
    bool is_clicked();
};

extern FrameObject * default_systembox_instance;

#endif // CHOWDREN_SYSTEMBOX_H
