/*
    File intentionally left blank
    This is used as a configuration file for the UMM malloc implementation we use
*/

#include <stdint.h>

// ... but these are required on some toolchains even if we don't use umm_init()
// (note that this is C)
static void *UMM_MALLOC_CFG_HEAP_ADDR = 0;
static uint32_t UMM_MALLOC_CFG_HEAP_SIZE = 0;

#define UMM_FIRST_FIT