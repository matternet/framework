#pragma once

#include <stdint.h>

// NOTE: in the CMSIS (stm32f3xx.h), this is #define used for the STM32F302xB
#define STM32F302xC

#define BOARD_PARAM1_FLASH_SIZE ((size_t)&_param1_flash_sec_end - (size_t)&_param1_flash_sec)
#define BOARD_PARAM2_FLASH_SIZE ((size_t)&_param2_flash_sec_end - (size_t)&_param2_flash_sec)

#define BOARD_PARAM1_ADDR (&_param1_flash_sec)
#define BOARD_PARAM2_ADDR (&_param2_flash_sec)

extern uint8_t _param1_flash_sec;
extern uint8_t _param1_flash_sec_end;
extern uint8_t _param2_flash_sec;
extern uint8_t _param2_flash_sec_end;

void board_get_unique_id(uint8_t* buf, uint8_t len);

#if !defined(_FROM_ASM_)
void boardInit(void);
#endif /* _FROM_ASM_ */
