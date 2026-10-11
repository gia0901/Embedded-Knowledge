/* lib_api.h — MAT TIEN C cong khai DUY NHAT cua libdisplay (ranh gioi MO).
 * Chi kieu C di qua day: khong class, khong std::string, khong exception. */
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define LIB_OK 0

int32_t lib_api_set_backlight(int32_t level);   /* PQ: tinh theo noi dung roi moi ghi */
int32_t lib_api_get_backlight(void);            /* doc state chung (shared memory)    */
int32_t lib_api_set_frequency(int32_t hz);      /* Panel Control: mot lenh, di thang  */

#ifdef __cplusplus
}
#endif
