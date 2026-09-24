#ifndef WII64_WIIU_GX2_FRAMEBUFFER_H
#define WII64_WIIU_GX2_FRAMEBUFFER_H

#include <stdint.h>

#include <gx2/texture.h>

class WiiUGX2Framebuffer {
public:
        WiiUGX2Framebuffer();
        ~WiiUGX2Framebuffer();

        bool resize(uint32_t width, uint32_t height);
        bool upload(const uint8_t *rdram, uint32_t address, uint32_t stride,
                    uint32_t width, uint32_t height, uint32_t pixelSize,
                    bool memoryByteSwapped);

        const GX2Texture *texture() const { return framebuffer; }
        uint32_t width() const { return framebufferWidth; }
        uint32_t height() const { return framebufferHeight; }

private:
        void destroy();

        GX2Texture *framebuffer;
        uint32_t framebufferWidth;
        uint32_t framebufferHeight;
};

#endif