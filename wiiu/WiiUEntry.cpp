#include <whb/gfx.h>

#include "WiiUPlatform.h"

int main(int, char **)
{
        if (!wiiu_platform_init())
                return 1;

        while (wiiu_platform_running()) {
                wiiu_platform_begin_frame();

                WHBGfxBeginRenderTV();
                WHBGfxClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                WHBGfxFinishRenderTV();

                wiiu_platform_end_frame();
        }

        wiiu_platform_shutdown();
        return 0;
}