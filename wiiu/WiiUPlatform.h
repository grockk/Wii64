#ifndef WII64_WIIU_PLATFORM_H
#define WII64_WIIU_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

int wiiu_platform_init(void);
int wiiu_platform_running(void);
void wiiu_platform_begin_frame(void);
void wiiu_platform_end_frame(void);
void wiiu_platform_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif