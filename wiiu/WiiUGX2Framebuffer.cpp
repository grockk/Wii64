#include "WiiUGX2Framebuffer.h"

#include <malloc.h>
#include <string.h>

#include <coreinit/memdefaultheap.h>
#include <gx2/mem.h>

static void initTexture(GX2Texture *texture, uint32_t width, uint32_t height)
{
        memset(texture, 0, sizeof(*texture));
        texture->surface.dim = GX2_SURFACE_DIM_TEXTURE_2D;
        texture->surface.width = width;
        texture->surface.height = height;
        texture->surface.depth = 1;
        texture->surface.mipLevels = 1;
        texture->surface.format = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
        texture->surface.aa = GX2_AA_MODE1X;
        texture->surface.use = GX2_SURFACE_USE_TEXTURE;
        texture->surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
        GX2CalcSurfaceSizeAndAlignment(&texture->surface);
        texture->viewFirstMip = 0;
        texture->viewNumMips = 1;
        texture->viewFirstSlice = 0;
        texture->viewNumSlices = 1;
        texture->compMap = 0x00010203;
        GX2InitTextureRegs(texture);
}

namespace {

static uint16_t read16(const uint8_t *rdram, uint32_t address,
                       bool memoryByteSwapped)
{
        if (memoryByteSwapped)
                return (uint16_t)(rdram[address ^ 2] << 8 | rdram[address ^ 3]);
        return (uint16_t)(rdram[address] << 8 | rdram[address + 1]);
}

static uint32_t read32(const uint8_t *rdram, uint32_t address,
                       bool memoryByteSwapped)
{
        uint32_t value;
        if (memoryByteSwapped) {
                value = (uint32_t)rdram[address ^ 3] << 24;
                value |= (uint32_t)rdram[address ^ 2] << 16;
                value |= (uint32_t)rdram[address ^ 1] << 8;
                value |= rdram[address];
                return value;
        }

        value = (uint32_t)rdram[address] << 24;
        value |= (uint32_t)rdram[address + 1] << 16;
        value |= (uint32_t)rdram[address + 2] << 8;
        return value | rdram[address + 3];
}

static uint8_t expand5(uint32_t value)
{
        return (uint8_t)((value << 3) | (value >> 2));
}

}

WiiUGX2Framebuffer::WiiUGX2Framebuffer()
        : framebuffer(nullptr), framebufferWidth(0), framebufferHeight(0)
{
}

WiiUGX2Framebuffer::~WiiUGX2Framebuffer()
{
        destroy();
}

void WiiUGX2Framebuffer::destroy()
{
        if (!framebuffer)
                return;

        if (framebuffer->surface.image)
                MEMFreeToDefaultHeap(framebuffer->surface.image);
        delete framebuffer;
        framebuffer = nullptr;
        framebufferWidth = 0;
        framebufferHeight = 0;
}

bool WiiUGX2Framebuffer::resize(uint32_t width, uint32_t height)
{
        GX2Texture *texture;

        if (framebuffer && framebufferWidth == width && framebufferHeight == height)
                return true;
        if (!width || !height)
                return false;

        destroy();
        texture = new GX2Texture();
        initTexture(texture, width, height);
        texture->surface.image = MEMAllocFromDefaultHeapEx(
                texture->surface.imageSize, texture->surface.alignment);
        if (!texture->surface.image) {
                delete texture;
                return false;
        }

        framebuffer = texture;
        framebufferWidth = width;
        framebufferHeight = height;
        return true;
}

bool WiiUGX2Framebuffer::upload(const uint8_t *rdram, uint32_t address,
                                uint32_t stride, uint32_t width,
                                uint32_t height, uint32_t pixelSize,
                                bool memoryByteSwapped)
{
        uint8_t *destination;
        uint32_t y;

        if (!rdram || !width || !height || (pixelSize != 2 && pixelSize != 4))
                return false;
        if (!resize(width, height))
                return false;

        destination = (uint8_t *)framebuffer->surface.image;
        for (y = 0; y < height; ++y) {
                uint8_t *row = destination + y * framebuffer->surface.pitch * 4;
                uint32_t x;
                for (x = 0; x < width; ++x) {
                        uint32_t source = address + y * stride + x * pixelSize;
                        uint8_t *pixel = row + x * 4;
                        if (pixelSize == 2) {
                                uint16_t value = read16(rdram, source,
                                                        memoryByteSwapped);
                                pixel[0] = expand5(value >> 11);
                                pixel[1] = expand5((value >> 6) & 0x1f);
                                pixel[2] = expand5((value >> 1) & 0x1f);
                                pixel[3] = (value & 1) ? 255 : 0;
                        } else {
                                uint32_t value = read32(rdram, source,
                                                         memoryByteSwapped);
                                pixel[0] = (uint8_t)(value >> 24);
                                pixel[1] = (uint8_t)(value >> 16);
                                pixel[2] = (uint8_t)(value >> 8);
                                pixel[3] = (uint8_t)value;
                        }
                }
        }

        GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE,
                      framebuffer->surface.image,
                      framebuffer->surface.imageSize);
        return true;
}