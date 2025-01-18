#pragma once

#define ROLE_TX 0
#define ROLE_RX 1

#if defined(ROLE)
    #if ROLE==ROLE_RX
        #define USE_OLED         // Приймач
    #elif ROLE==ROLE_TX
        #define USE_ACCEL           // Передавач
    #else
        #error ROLE must by defined TX or RX
    #endif
#else
    #error ROLE must by defined ROLE_TX or ROLE_RX
#endif
