
#include "printf.h"

#if USE_OS
    #include "os.h"
#endif

/// Консольный ввод/вывод
#ifdef DEBUG
    #define USR_PUT_RTT 1 ///< Вывод консоли в RTT
    #define USR_PUT_ITM 0 ///< Вывод консоли в SWO
    #define USR_GET_RTT 0 ///< Консольный ввод из RTT
    #define USR_GET_ITM 0 ///< Консольный ввод из SWO
#else
    #define USR_PUT_RTT 0 ///< Вывод консоли в RTT
    #define USR_PUT_ITM 0 ///< Вывод консоли в SWO
    #define USR_GET_RTT 0 ///< Консольный ввод из RTT
    #define USR_GET_ITM 0 ///< Консольный ввод из SWO
#endif

#if USR_PUT_RTT || USR_GET_RTT
    #include "SEGGER_RTT.h"
#endif


#if USR_PUT_ITM || USR_GET_ITM
    #include "RTE_Components.h"
    #include CMSIS_device_header
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
    ITM_SendChar(_ch);
#endif
}

#endif // USR_PUT_RTT || USR_PUT_ITM


#if USR_GET_RTT || USR_GET_ITM

int stdin_getchar(void)
{
    #if (USR_GET_RTT == 0) && (USR_GET_ITM == 0)
        return -1;
    #else
        int result = -1;

        do
        {

        #if (USR_GET_RTT != 0)
            if (result == -1) result = SEGGER_RTT_GetKey();
        #endif
        
        #if (USR_GET_ITM != 0)
            if (result == -1) result = ITM_ReceiveChar();
        #endif
        
        #ifdef USE_OS
            if (result == -1) os::task_base::yield();
        #endif
        }
        while (result == -1);

        return result;
    #endif
}
#endif // USR_GET_RTT || USR_GET_ITM
