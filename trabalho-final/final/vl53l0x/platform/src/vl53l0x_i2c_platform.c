
/*
 * COPYRIGHT (C) STMicroelectronics 2015. All rights reserved.
 *
 * This software is the confidential and proprietary information of
 * STMicroelectronics ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with STMicroelectronics
 *
 * Programming Golden Rule: Keep it Simple!
 *
 */

 * \file   VL53L0X_platform.c
 * \brief  Code function defintions for Ewok Platform Layer
 *
 */

 // #include <windows.h>
#include <stdio.h> // sprintf(), vsnprintf(), printf()

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#include <vl53l0x/platform/inc/vl53l0x_i2c_platform.h>
#include <vl53l0x/core/inc/vl53l0x_def.h>

 // #include "ranging_sensor_comms.h"
 // #include "comms_platform.h"

#include <vl53l0x/platform/inc/vl53l0x_platform_log.h>

#ifdef VL53L0X_LOG_ENABLE
#define trace_print(level, ...) trace_print_module_function(TRACE_MODULE_PLATFORM, level, TRACE_FUNCTION_NONE, ##__VA_ARGS__)
#define trace_i2c(...) trace_print_module_function(TRACE_MODULE_NONE, TRACE_LEVEL_NONE, TRACE_FUNCTION_I2C, ##__VA_ARGS__)
#endif

#include <msp430.h>

char debug_string[VL53L0X_MAX_STRING_LENGTH_PLT];

#define MIN_COMMS_VERSION_MAJOR 1
#define MIN_COMMS_VERSION_MINOR 8
#define MIN_COMMS_VERSION_BUILD 1
#define MIN_COMMS_VERSION_REVISION 0

#define STATUS_OK 0x00
#define STATUS_FAIL 0x01

 bool_t _check_min_version(void)
 {
     return 0;
 }

 int32_t VL53L0X_comms_initialise(uint8_t comms_type, uint16_t comms_speed_khz)
 {

     int32_t status = STATUS_OK;
     //    i2cInit(B0, 1, 10000, 0);
     return status;
 }

 int32_t VL53L0X_comms_close(void)
 {
     int32_t status = STATUS_OK;
     return status;
 }

#define I2C_TIMEOUT 10000

 int32_t VL53L0X_write_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
 {
     int32_t i;

     UCB0I2CSA = address;

     UCB0CTL1 |= UCTR | UCTXSTT;

     while (!(UCB0IFG & UCTXIFG))
         ;

     UCB0TXBUF = index;

     while (UCB0CTL1 & UCTXSTT)
         ;

     if (UCB0IFG & UCNACKIFG)
     {
         UCB0CTL1 |= UCTXSTP;
         while (UCB0CTL1 & UCTXSTP)
             ;
         return STATUS_FAIL;
     }

     for (i = 0; i < count; i++)
     {
         while (!(UCB0IFG & UCTXIFG))
             ;
         UCB0TXBUF = pdata[i];
     }

     while (!(UCB0IFG & UCTXIFG))
         ;

     UCB0CTL1 |= UCTXSTP;
     while (UCB0CTL1 & UCTXSTP)
         ;

     return STATUS_OK;
 }

 int32_t VL53L0X_read_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
 {
     int32_t i;

     UCB0I2CSA = address;
     UCB0CTL1 |= UCTR | UCTXSTT; // Modo transmissor, gera START
     while (!(UCB0IFG & UCTXIFG))
         ;
     UCB0TXBUF = index;
     while (UCB0CTL1 & UCTXSTT)
         ;

     UCB0CTL1 &= ~UCTR;
     UCB0CTL1 |= UCTXSTT;
     while (UCB0CTL1 & UCTXSTT)
         ;

     if (count == 1)
     {

         UCB0CTL1 |= UCTXSTP;
         while (!(UCB0IFG & UCRXIFG))
             ;
         pdata[0] = UCB0RXBUF;
     }
     else
     {

         for (i = 0; i < count - 1; i++)
         {
             while (!(UCB0IFG & UCRXIFG))
                 ;
             pdata[i] = UCB0RXBUF;
             if (i == count - 2)
             {

                 UCB0CTL1 |= UCTXSTP;
             }
         }

         while (!(UCB0IFG & UCRXIFG))
             ;
         pdata[count - 1] = UCB0RXBUF;
     }

     while (UCB0CTL1 & UCTXSTP)
         ;

     return STATUS_OK;
 }

 int32_t VL53L0X_write_byte(uint8_t address, uint8_t index, uint8_t data)
 {
     int32_t status = STATUS_OK;
     const int32_t cbyte_count = 1;

     status = VL53L0X_write_multi(address, index, &data, cbyte_count);

     return status;
 }

 int32_t VL53L0X_write_word(uint8_t address, uint8_t index, uint16_t data)
 {
     int32_t status = STATUS_OK;

     uint8_t buffer[BYTES_PER_WORD];

     // Split 16-bit word into MS and LS uint8_t
     buffer[0] = (uint8_t)(data >> 8);
     buffer[1] = (uint8_t)(data & 0x00FF);

     status = VL53L0X_write_multi(address, index, buffer, BYTES_PER_WORD);

     return status;
 }

 int32_t VL53L0X_write_dword(uint8_t address, uint8_t index, uint32_t data)
 {
     int32_t status = STATUS_OK;
     uint8_t buffer[BYTES_PER_DWORD];

     // Split 32-bit word into MS ... LS bytes
     buffer[0] = (uint8_t)(data >> 24);
     buffer[1] = (uint8_t)((data & 0x00FF0000) >> 16);
     buffer[2] = (uint8_t)((data & 0x0000FF00) >> 8);
     buffer[3] = (uint8_t)(data & 0x000000FF);

     status = VL53L0X_write_multi(address, index, buffer, BYTES_PER_DWORD);

     return status;
 }

 int32_t VL53L0X_read_byte(uint8_t address, uint8_t index, uint8_t *pdata)
 {
     int32_t status = STATUS_OK;
     int32_t cbyte_count = 1;

     status = VL53L0X_read_multi(address, index, pdata, cbyte_count);

     return status;
 }

 int32_t VL53L0X_read_word(uint8_t address, uint8_t index, uint16_t *pdata)
 {
     int32_t status = STATUS_OK;
     uint8_t buffer[BYTES_PER_WORD];

     status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_WORD);
     *pdata = ((uint16_t)buffer[0] << 8) + (uint16_t)buffer[1];

     return status;
 }

 int32_t VL53L0X_read_dword(uint8_t address, uint8_t index, uint32_t *pdata)
 {
     int32_t status = STATUS_OK;
     uint8_t buffer[BYTES_PER_DWORD];

     status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_DWORD);
     *pdata = ((uint32_t)buffer[0] << 24) + ((uint32_t)buffer[1] << 16) + ((uint32_t)buffer[2] << 8) + (uint32_t)buffer[3];

     return status;
 }

 int32_t VL53L0X_platform_wait_us(int32_t wait_us)
 {
     int32_t status = STATUS_OK;

     return status;
 }

 int32_t VL53L0X_wait_ms(int32_t wait_ms)
 {
     int32_t status = STATUS_OK;

     return status;
 }

 int32_t VL53L0X_set_gpio(uint8_t level)
 {
     int32_t status = STATUS_OK;
     // status = VL53L0X_set_gpio_sv(level);
#ifdef VL53L0X_LOG_ENABLE
     trace_i2c("// Set GPIO = %d;\n", level);
#endif

     return status;
 }

 int32_t VL53L0X_get_gpio(uint8_t *plevel)
 {
     int32_t status = STATUS_OK;
#ifdef VL53L0X_LOG_ENABLE
     trace_i2c("// Get GPIO = %d;\n", *plevel);
#endif
     return status;
 }

 int32_t VL53L0X_release_gpio(void)
 {
     int32_t status = STATUS_OK;
#ifdef VL53L0X_LOG_ENABLE
     trace_i2c("// Releasing force on GPIO\n");
#endif
     return status;
 }

 int32_t VL53L0X_cycle_power(void)
 {
     int32_t status = STATUS_OK;
#ifdef VL53L0X_LOG_ENABLE
     trace_i2c("// cycle sensor power\n");
#endif
     //    status = RANGING_SENSOR_COMMS_Cycle_Sensor_Power();

     if (status != STATUS_OK)
     {
         //        RANGING_SENSOR_COMMS_Get_Error_Text(debug_string);
     }

     return status;
 }

 int32_t VL53L0X_get_timer_frequency(int32_t *ptimer_freq_hz)
 {
     *ptimer_freq_hz = 0;
     return STATUS_FAIL;
 }

 int32_t VL53L0X_get_timer_value(int32_t *ptimer_count)
 {
     *ptimer_count = 0;
     return STATUS_FAIL;
 }
