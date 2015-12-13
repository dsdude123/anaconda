#include "debugdraw.h"

#ifdef USE_DEBUGDRAW

#include "render.h"
#include "types.h"

static const unsigned long long font6x8[] =
{
    0x3E00000000000000,0x6F6B3E003E455145,0x1C3E7C3E1C003E6B,0x3000183C7E3C1800,
    0x7E5C180030367F36,0x000018180000185C,0x0000FFFFE7E7FFFF,0xDBDBC3FF00000000,
    0x0E364A483000FFC3,0x6000062979290600,0x0A7E600004023F70,0x2A1C361C2A003F35,
    0x0800081C3E7F0000,0x7F361400007F3E1C,0x005F005F00001436,0x22007F017F090600,
    0x606060002259554D,0x14B6FFB614000060,0x100004067F060400,0x3E08080010307F30,
    0x08083E1C0800081C,0x0800404040407800,0x3F3C3000083E083E,0x030F3F0F0300303C,
    0x0000000000000000,0x0003070000065F06,0x247E247E24000307,0x630000126A2B2400,
    0x5649360063640813,0x0000030700005020,0x00000000413E0000,0x1C3E080000003E41,
    0x08083E080800083E,0x0800000060E00000,0x6060000008080808,0x0204081020000000,
    0x00003E4549513E00,0x4951620000407F42,0x3649494922004649,0x2F00107F12141800,
    0x494A3C0031494949,0x0305097101003049,0x0600364949493600,0x6C6C00001E294949,
    0x00006CEC00000000,0x2400004122140800,0x2241000024242424,0x0609590102000814,
    0x7E001E555D413E00,0x49497F007E111111,0x224141413E003649,0x7F003E4141417F00,
    0x09097F0041494949,0x7A4949413E000109,0x00007F0808087F00,0x4040300000417F41,
    0x412214087F003F40,0x7F00404040407F00,0x04027F007F020402,0x3E4141413E007F08,
    0x3E00060909097F00,0x09097F005E215141,0x3249494926006619,0x3F0001017F010100,
    0x40201F003F404040,0x3F403C403F001F20,0x0700631408146300,0x4549710007087008,
    0x0041417F00000043,0x0000201008040200,0x01020400007F4141,0x8080808080800402,
    0x2000000007030000,0x44447F0078545454,0x2844444438003844,0x38007F4444443800,
    0x097E080008545454,0x7CA4A4A418000009,0x0000007804047F00,0x8480400000407D00,
    0x004428107F00007D,0x7C0000407F000000,0x04047C0078041804,0x3844444438000078,
    0x380038444444FC00,0x44784400FC444444,0x2054545408000804,0x3C000024443E0400,
    0x40201C00007C2040,0x3C6030603C001C20,0x9C00006C10106C00,0x54546400003C60A0,
    0x0041413E0800004C,0x0000000077000000,0x02010200083E4141,0x3C2623263C000001,
    0x3D001221E1A11E00,0x54543800007D2040,0x7855555520000955,0x2000785554552000,
    0x5557200078545555,0x1422E2A21C007857,0x3800085555553800,0x5555380008555455,
    0x00417C0100000854,0x0000004279020000,0x2429700000407C01,0x782F252F78007029,
    0x3400455554547C00,0x7F097E0058547C54,0x0039454538004949,0x3900003944453800,
    0x21413C0000384445,0x007C20413D00007D,0x3D00003D60A19C00,0x40413C00003D4242,
    0x002466241800003D,0x29006249493E4800,0x16097F00292A7C2A,0x02097E8840001078,
    0x0000785555542000,0x4544380000417D00,0x007D21403C000039,0x7A0000710A097A00,
    0x5555080000792211,0x004E51514E005E55,0x3C0020404D483000,0x0404040404040404,
    0x506A4C0817001C04,0x0000782A34081700,0x0014080000307D30,0x0814000814001408,
    0x55AA114411441144,0xEEBBEEBB55AA55AA,0x0000FF000000EEBB,0x0A0A0000FF080808,
    0xFF00FF080000FF0A,0x0000F808F8080000,0xFB0A0000FE0A0A0A,0xFF00FF000000FF00,
    0x0000FE02FA0A0000,0x0F0800000F080B0A,0x0F0A0A0A00000F08,0x0000F80808080000,
    0x080808080F000000,0xF808080808080F08,0x0808FF0000000808,0x0808080808080808,
    0xFF0000000808FF08,0x0808FF00FF000A0A,0xFE000A0A0B080F00,0x0B080B0A0A0AFA02,
    0x0A0AFA02FA0A0A0A,0x0A0A0A0AFB00FF00,0xFB00FB0A0A0A0A0A,0x0A0A0B0A0A0A0A0A,
    0x0A0A08080F080F08,0xF808F8080A0AFA0A,0x08080F080F000808,0x00000A0A0F000000,
    0xF808F8000A0AFE00,0x0808FF00FF080808,0x08080A0AFB0A0A0A,0xF800000000000F08,
    0xFFFFFFFFFFFF0808,0xFFFFF0F0F0F0F0F0,0xFF000000000000FF,0x0F0F0F0F0F0FFFFF,
    0xFE00241824241800,0x01017F0000344A4A,0x027E027E02000003,0x1800006349556300,
    0x2020FC00041C2424,0x000478040800001C,0x3E00085577550800,0x02724C00003E4949,
    0x0030595522004C72,0x1800182418241800,0x2A2A1C0018247E24,0x003C02023C00002A,
    0x0000002A2A2A2A00,0x4A4A510000242E24,0x00514A4A44000044,0x20000402FC000000,
    0x2A08080000003F40,0x0012241224000808,0x0000000609090600,0x0008000000001818,
    0x02023E4030000000,0x0900000E010E0100,0x3C3C3C0000000A0D,0x000000000000003C
};

static bool font_init = false;
static Texture font_tex[256];

#define FONT_X 6
#define FONT_Y 8

void setup_font()
{
    unsigned char * font;
    const unsigned long long * font_s = &font6x8[0];
    memcpy(&font, &font_s, sizeof(unsigned char*));
    for (unsigned int i = 0; i < 256; ++i) {
        unsigned char * vv = &font[i*6];
        unsigned long long * p;
        memcpy(&p, &vv, sizeof(unsigned long long*));
        unsigned long long v = *p;
        unsigned char data[FONT_X * FONT_Y];
        for (unsigned int ii = 0; ii < FONT_X * FONT_Y; ++ii) {
            unsigned char & c = data[ii];
            unsigned int y = ii / FONT_X;
            unsigned int x = ii % FONT_X;
            if (v & (1ULL << (y + x * FONT_Y)))
                c = 255;
            else
                c = 0;
        }

        font_tex[i] = Render::create_tex(&data[0], Render::L, FONT_X, FONT_Y);
        Render::set_filter(font_tex[i], false);
    }
}

static vector<chowstring> lines;

void Debug::print(const chowstring & s)
{
    lines.push_back(s);
}

#define DEBUG_X 25
#define DEBUG_Y 25
#define X_SPACING FONT_X
#define Y_SPACING (FONT_Y + 5)

void Debug::draw()
{
    if (lines.empty())
        return;

    if (!font_init) {
        setup_font();
        font_init = true;
    }

    vector<chowstring>::iterator it;
    unsigned int y = DEBUG_Y;
    for (unsigned int i = 0; i < lines.size(); ++i) {
        chowstring & s = lines[i];
        int y_off = i == (lines.size() - 1) ? 0 : -5;

        unsigned int x = DEBUG_X;
        Render::draw_quad(DEBUG_X - 5, y - 5,
                          x + X_SPACING * s.size() + 5,
                          y + Y_SPACING + y_off,
                          Color(0, 0, 0, 128));

        Render::set_effect(Render::FONT);
        for (unsigned int i = 0; i < s.size(); ++i) {
            unsigned char c = s[i];

            Texture t = font_tex[c];
            Render::draw_tex(x, y, x + FONT_X, y + FONT_Y,
                             Color(255, 255, 255, 255), t);

            x += X_SPACING;
        }
        Render::disable_effect();

        y += Y_SPACING;
    }


    lines.clear();
}

#endif
