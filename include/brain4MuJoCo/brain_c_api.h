#pragma once

#include <stdint.h>

#if defined(_WIN32)
#if defined(WORM_BRAIN_BUILD_DLL)
#define WORM_BRAIN_API __declspec(dllexport)
#else
#define WORM_BRAIN_API __declspec(dllimport)
#endif
#else
#define WORM_BRAIN_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

WORM_BRAIN_API int32_t worm_brain_init(const char* source_dir);

WORM_BRAIN_API void worm_brain_reset(void);

WORM_BRAIN_API float* simulate(int32_t step);

WORM_BRAIN_API int32_t worm_brain_muscle_count(void);

WORM_BRAIN_API const char* worm_brain_last_error(void);

WORM_BRAIN_API void worm_brain_shutdown(void);

#ifdef __cplusplus
}
#endif