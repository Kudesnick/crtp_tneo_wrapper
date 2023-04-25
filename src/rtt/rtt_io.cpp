
#include "RTE_Components.h"
#include CMSIS_device_header
#include <stdio.h>

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


#if defined(RTE_Compiler_IO_TTY) || defined(RTE_Compiler_IO_STDOUT) || defined(RTE_Compiler_IO_STDERR)
#if defined(RTE_Compiler_IO_TTY_User) || defined(RTE_Compiler_IO_STDOUT_User) || defined(RTE_Compiler_IO_STDERR_User)

extern "C" int usr_put_char(int);
extern "C" void usr_put_str(const unsigned char *, uint32_t);

#ifdef RTE_Compiler_IO_TTY_User
    extern "C" void ttywrch(int);
#endif
#ifdef RTE_Compiler_IO_STDOUT_User
    extern "C" int stdout_putchar(int);
#endif
#ifdef RTE_Compiler_IO_STDERR_User
    extern "C" int stderr_putchar(int);
#endif

#endif
#endif


#if defined(RTE_Compiler_IO_TTY) || defined(RTE_Compiler_IO_STDIN)
#if defined(RTE_Compiler_IO_TTY_User) || defined(RTE_Compiler_IO_STDIN_User)

extern "C" int stdin_getchar(void);

#endif
#endif


#if defined(RTE_Compiler_IO_TTY) || defined(RTE_Compiler_IO_STDOUT) || defined(RTE_Compiler_IO_STDERR)
#if defined(RTE_Compiler_IO_TTY_User) || defined(RTE_Compiler_IO_STDOUT_User) || defined(RTE_Compiler_IO_STDERR_User)
        
int usr_put_char(int ch)
{
    #if (USR_PUT_RTT == 0) && (USR_PUT_ITM == 0)
        (void)ch;
        
        return -1;
    #else
        
        int result = -1;
        
        #if (USR_PUT_RTT != 0)
            result = static_cast<int>(SEGGER_RTT_PutChar(0, static_cast<char>(ch)));
        #endif
        
        #if (USR_PUT_ITM != 0)
            if ((ITM_TCR & ITM_TCR_ITMENA_Msk) && /* ITM enabled */
                (ITM_TER & (1UL << 0)        ))   /* ITM Port #0 enabled */
            {
                while (ITM_PORT0_U32 == 0);
                __NOP();
                ITM_PORT0_U8 = static_cast<uint8_t>(ch);
            }
            result = ch;
        #endif
        
        return result;
    #endif
}

void usr_put_str(const unsigned char *_buf, uint32_t _len)
{
    #if (USR_PUT_RTT == 0) && (USR_PUT_ITM == 0)
        (void)_buf;
        (void)_len;
    #else
        
        #if (USR_PUT_RTT != 0)
            SEGGER_RTT_Write(0, _buf, _len);
        #endif
        
        #if (USR_PUT_ITM != 0)
            for (; _len; _len--)
            {
                char ch = *_buf++;

                if ((ITM_TCR & ITM_TCR_ITMENA_Msk) && /* ITM enabled */
                    (ITM_TER & (1UL << 0)        ))   /* ITM Port #0 enabled */
                {
                    while (ITM_PORT0_U32 == 0);
                    __NOP();
                    ITM_PORT0_U8 = static_cast<uint8_t>(ch);
                }
            }
        #endif
    #endif
}

#ifdef RTE_Compiler_IO_TTY_User
    void ttywrch(int ch)
    {
        usr_put_char(ch);
    }
#endif

#ifdef RTE_Compiler_IO_STDOUT_User
    int stdout_putchar(int ch)
    {    
        return usr_put_char(ch);
    }
#endif

#ifdef RTE_Compiler_IO_STDERR_User
    int stderr_putchar(int ch)
    {
        return usr_put_char(ch);
    }
#endif

#endif
#endif


#if defined(RTE_Compiler_IO_TTY) || defined(RTE_Compiler_IO_STDIN)
#if defined(RTE_Compiler_IO_TTY_User) || defined(RTE_Compiler_IO_STDIN_User)
    
#if (!defined(RTE_Compiler_IO_TTY_ITM)    && \
     !defined(RTE_Compiler_IO_STDIN_ITM)  && \
     !defined(RTE_Compiler_IO_STDOUT_ITM) && \
     !defined(RTE_Compiler_IO_STDERR_ITM) && \
     (USR_GET_ITM != 0))
    
    volatile int32_t ITM_RxBuffer = ITM_RXBUFFER_EMPTY;

#endif

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
#endif
