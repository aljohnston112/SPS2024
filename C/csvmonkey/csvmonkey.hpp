#if defined(__SSE4_2__)
#define CAN_USE_SSE42
#include <emmintrin.h>
#include <smmintrin.h>
#endif // __SSE4_2__