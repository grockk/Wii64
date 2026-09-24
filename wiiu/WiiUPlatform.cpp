#include "WiiUPlatform.h"

#include <vpad/input.h>
#include <whb/gfx.h>
#include <whb/proc.h>

static bool platform_initialized = false;

int wiiu_platform_init(void)
{
        WHBProcInit();
        if (!WHBGfxInit()) {
                WHBProcShutdown();
                return 0;
        }
        VPADInit();

        platform_initialized = true;
        return 1;
}


int wiiu_platform_running(void)
{
        return platform_initialized && WHBProcIsRunning();
}

void wiiu_platform_begin_frame(void)
{
        if (platform_initialized)
                WHBGfxBeginRender();
}

void wiiu_platform_end_frame(void)
{
        if (platform_initialized)
                WHBGfxFinishRender();
}

void wiiu_platform_shutdown(void)
{
        if (!platform_initialized)
                return;

        VPADShutdown();
        WHBGfxShutdown();
        WHBProcShutdown();
        platform_initialized = false;
}