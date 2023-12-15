#include <stdint.h>
#include <lcom/lcf.h>
#include "rtc.h"

/// @brief Variable used for storing the hook_id of the rtc interrupts.
int hook_id_rtc = 4;

/// @brief Struct used for storing the current time.
static real_time_info time_info;

/// @brief Variable that determines if the rtc is in binary mode or not.
static uint8_t is_binary;

/// @brief Initializes the rtc, either in binary or BCD mode.
/// @return Updates the time_info struct with the current time. Returns 0 upon success and non-zero otherwise.
int initialize_rtc(){
  is_binary = is_rtc_binary();
  return update_time();
}

/// @brief Subscribes and enables RTC interrupts.
/// @param bit_no Address of memory to be initialized with the bit number to be set in the mask.
/// @return Returns 0 upon success and non-zero otherwise.
int rtc_subscribe_int(uint8_t *bit_no){
  *bit_no = hook_id_rtc;

  if(sys_irqsetpolicy(RTC_IRQ, IRQ_REENABLE, &hook_id_rtc) != OK){
    printf("Error in sys_irqsetpolicy\n");
    return 1;
  }

  return OK;
}

/// @brief Unsubscribes RTC interrupts.
/// @return Returns 0 upon success and non-zero otherwise.
int rtc_unsubscribe_int(){
  if(sys_irqrmpolicy(&hook_id_rtc) != OK){
    printf("Error in sys_irqrmpolicy\n");
    return 1;
  }

  return OK;
}

/// @brief Reads the output of the RTC.
/// @param command Command byte to be written in register 0x70.
/// @param output Variable to be updated with the output of the RTC.
/// @return Returns 0 upon success and non-zero otherwise.
int rtc_read_output(uint8_t command, uint8_t *output){
  if(sys_outb(0x70, command) != OK){
    return 1;
  }
  if(util_sys_inb(0x71, output) != OK) return 1;

  return OK;
}

/// @brief Checks if the RTC is updating.
/// @return Returns true if the RTC is updating and false otherwise.
bool is_rtc_updating(){
  uint8_t result;

  if(rtc_read_output(REG_A, &result) != OK) return 1;

  // If bit 7 is up it means that the rtc is updating.
  return ((result & BIT(7)) != 0);
}

/// @brief Checks if the RTC is in binary mode.
/// @return Returns 0 if the RTC is in BCD mode and 1 if it is in binary mode.
bool is_rtc_binary(){
  uint8_t result;

  if(rtc_read_output(REG_B, &result) != OK) return 1;

  return (result & BIT(2));
}

/// @brief Converts a BCD number to binary.
/// @param bcd BCD number to be converted.
/// @return Corresponding binary number.
uint8_t bcd_to_binary(uint8_t bcd){
  return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}


/// @brief Updates de RTC with the current time. The time_info struct is updated with the current time.
/// @return Returns 0 upon success and non-zero otherwise.
int update_time(){
  printf("Updating time...\n");
  //if rtc is updating do not update time
  if(is_rtc_updating()) return 1;
  printf("Time updated\n");

  uint8_t output;

  if(rtc_read_output(SECONDS, &output) != OK) return 1;
  time_info.seconds = is_binary ? output : bcd_to_binary(output);
  printf("Seconds: %x\n", time_info.seconds);

  if(rtc_read_output(MINUTES, &output) != OK) return 1;
  time_info.minutes = is_binary ? output : bcd_to_binary(output);

  if(rtc_read_output(HOURS, &output) != OK) return 1;
  time_info.hours = is_binary ? output : bcd_to_binary(output);

  if(rtc_read_output(DAYS, &output) != OK) return 1;
  time_info.day = is_binary ? output : bcd_to_binary(output);

  if(rtc_read_output(MONTHS, &output) != OK) return 1;
  time_info.month = is_binary ? output : bcd_to_binary(output);

  if(rtc_read_output(YEARS, &output) != OK) return 1;
  time_info.year = is_binary ? output : bcd_to_binary(output);

  //print time
  printf("%d/%d/%d %d:%d:%d\n", time_info.day, time_info.month, time_info.year, time_info.hours, time_info.minutes, time_info.seconds);

  return OK;
}

/// @brief RTC interrupt handler.
/// Reads the output of register C and updates the time if the interrupt was generated by the RTC.
/// @return Returns 0 upon success and non-zero otherwise.
int rtc_ih() {
  uint8_t data;

  if (rtc_read_output(REG_C, &data) != OK) {
    printf("Error in rtc_read_output\n");
    return 1;
  }

  if (data & BIT(4)) {
    update_time();
  }
  return OK;
}

/// @brief Formats the date and time into a string, showing the full date and time.
/// @param date_time String to be updated with the date and time.
/// @return Returns 0 upon success and non-zero otherwise.
int get_full_date_time(char* date_time) {
  if (update_time() != OK) {
    printf("Failed to update the time.\n");
    return 1;
  }

  sprintf(date_time, "%d/%d/%d %d:%d:%d", time_info.day, time_info.month, time_info.year, time_info.hours, time_info.minutes, time_info.seconds);
  return OK;
}

/// @brief Formats the date and time into a string, showing the date and time until the minutes.
/// @param date_time String to be updated with the date and time.
/// @return Returns 0 upon success and non-zero otherwise.
int get_time_until_minutes(char* date_time) {
  if (update_time() != OK) {
    printf("Failed to update the time.\n");
    return 1;
  }

  sprintf(date_time, "%d/%d/%d %d:%d", time_info.day, time_info.month, time_info.year, time_info.hours, time_info.minutes);
  return OK;
}