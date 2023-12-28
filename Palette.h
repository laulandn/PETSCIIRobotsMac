#ifndef _PRPALETTE_H
#define _PRPALETTE_H

#include "Platform.h"

class PRPalette {
public:
    static void initialize();

    PRPalette(const uint16_t* palette = 0, uint16_t colorCount = 0, uint16_t fade = 15, uint16_t fadeBaseColor = 0x000);
    ~PRPalette();

    void setPRPalette(const uint16_t* palette, uint16_t colorCount);
    void setFade(uint16_t fade);
    __inline uint16_t fade() const;
    void setFadeBaseColor(uint16_t fadeBaseColor);
    __inline uint16_t* palette() const;

private:
    void update();

    uint16_t* sourcePRPalette;
    uint16_t* currentPRPalette;
    uint16_t colorCount_;
    uint16_t fade_;
    uint16_t fadeBaseColor;
};

#endif
