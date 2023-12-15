#ifndef RTC_H
#define RTC_H 

#include <stdint.h>
#include <lcom/lcf.h>

#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71

#define RTC_IRQ 8

#define SECONDS 0
#define MINUTES 2
#define HOURS 4
#define DAYS 7
#define MONTHS 8
#define YEARS 9

#define REG_A 10
#define REG_B 11
#define REG_C 12
#define REG_D 13

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} real_time_info;

int initialize_rtc();

int rtc_subscribe_int(uint8_t *bit_no);

int rtc_unsubscribe_int();

int rtc_read_output(uint8_t command, uint8_t *output);

bool is_rtc_updating();

bool is_rtc_binary();

uint8_t bcd_to_binary(uint8_t bcd);

int update_time();

int rtc_ih();

int get_full_date_time(char* date_time);

int get_time_until_minutes(char* date_time);

#endif
