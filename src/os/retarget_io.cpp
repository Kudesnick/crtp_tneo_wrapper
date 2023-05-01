
#include "printf.h"

#define USE_OS 1

#if USE_OS
#   include "os.h"
#endif

/// Консольный ввод/вывод
#ifdef DEBUG
    #define USR_PUT_RTT 1 ///< Вывод консоли в RTT
    #define USR_PUT_ITM 0 ///< Вывод консоли в SWO
    #define USR_GET_RTT 1 ///< Консольный ввод из RTT
    #define USR_GET_ITM 0 ///< Консольный ввод из SWO
#else
    #define USR_PUT_RTT 0 ///< Вывод консоли в RTT
    #define USR_PUT_ITM 0 ///< Вывод консоли в SWO
    #define USR_GET_RTT 0 ///< Консольный ввод из RTT
    #define USR_GET_ITM 0 ///< Консольный ввод из SWO
#endif

#if (USR_PUT_RTT == 1) && (USR_GET_RTT == 1)
    #include "SEGGER_RTT.h"
#endif


#if (USR_PUT_ITM != 0) || (USR_GET_ITM != 0)
    /* ITM registers */
    #define ITM_PORT0_U8          (*reinterpret_cast<volatile uint8_t  *>(0xE0000000))
    #define ITM_PORT0_U32         (*reinterpret_cast<volatile uint32_t *>(0xE0000000))
    #define ITM_TER               (*reinterpret_cast<volatile uint32_t *>(0xE0000E00))
    #define ITM_TCR               (*reinterpret_cast<volatile uint32_t *>(0xE0000E80))
    #ifndef ITM_TCR_ITMENA_Msk
    #define ITM_TCR_ITMENA_Msk    (1UL << 0)
    #endif
#endif


#if USR_PUT_RTT || USR_PUT_ITM

#if USE_OS

static os::mutex printf_mutex;

void _printf_mutex_acquire(void)
{
    printf_mutex.acquire(os::wait::infinitely);
}

void _printf_mutex_release(void)
{
    printf_mutex.release();
}

#endif // USE_OS

void _putchar(char _ch)
{
#if (USR_PUT_RTT != 0)
    SEGGER_RTT_PutChar(0, _ch);
#endif

#if (USR_PUT_ITM != 0)
    if ((ITM_TCR & ITM_TCR_ITMENA_Msk) && /* ITM enabled */
        (ITM_TER & (1UL << 0)        ))   /* ITM Port #0 enabled */
    {
        while (ITM_PORT0_U32 == 0);
        ITM_PORT0_U8 = 0;
        ITM_PORT0_U8 = _ch;
    }
#endif
}

#endif // USR_PUT_RTT || USR_PUT_ITM


#if (0)
int stdin_getchar(void)
{
    #if (USR_GET_RTT == 0) && (USR_GET_ITM == 0)
        return -1;
    #else
        int result = -1;
        
        do
        {

        #if (USR_GET_RTT != 0)
            result = SEGGER_RTT_GetKey();
        #endif
        
        #if (USR_GET_ITM != 0)
            if (result == -1)
            {
                extern volatile int32_t ITM_RxBuffer;
                
                if (ITM_RxBuffer != ITM_RXBUFFER_EMPTY)
                {
                    result = ITM_RxBuffer;
                    ITM_RxBuffer = ITM_RXBUFFER_EMPTY;  /* ready for next character */
                }
            }
        #endif
        
        #ifdef USE_OS
            if (result == -1)
            {
                os::task_base::yield();
            }
        #endif
        }
        while (result == -1);
        
        return result;
    #endif
}
#endif
