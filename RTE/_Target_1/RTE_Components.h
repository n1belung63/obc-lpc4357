
/*
 * Auto generated Run-Time-Environment Configuration File
 *      *** Do not modify ! ***
 *
 * Project: 'm4f' 
 * Target:  'Target 1' 
 */

#ifndef RTE_COMPONENTS_H
#define RTE_COMPONENTS_H


/*
 * Define the Device Header File: 
 */
#define CMSIS_device_header "LPC43xx.h"

/* ARM.FreeRTOS::RTOS:Config:FreeRTOS:10.3.1 */
#define RTE_RTOS_FreeRTOS_CONFIG        /* RTOS FreeRTOS Config for FreeRTOS API */
/* ARM.FreeRTOS::RTOS:Core:Cortex-M:10.3.1 */
#define RTE_RTOS_FreeRTOS_CORE          /* RTOS FreeRTOS Core */
/* ARM.FreeRTOS::RTOS:Heap:Heap_4:10.3.1 */
#define RTE_RTOS_FreeRTOS_HEAP_4        /* RTOS FreeRTOS Heap 4 */
/* Keil::CMSIS Driver:I2C:2.8 */
#define RTE_Drivers_I2C0                /* Driver I2C0 */
        #define RTE_Drivers_I2C1                /* Driver I2C1 */
/* Keil::CMSIS Driver:SPI:SSP:2.12 */
#define RTE_Drivers_SPI0                /* Driver SPI0 */
        #define RTE_Drivers_SPI1                /* Driver SPI1 */
/* Keil::CMSIS Driver:USART:2.17 */
#define RTE_Drivers_USART0              /* Driver USART0 */
        #define RTE_Drivers_USART1              /* Driver USART1 */
        #define RTE_Drivers_USART2              /* Driver USART2 */
        #define RTE_Drivers_USART3              /* Driver USART3 */
/* Keil::Device:Startup:1.0.0 */
#define RTE_DEVICE_STARTUP_LPC43XX      /* Device Startup for NXP43XX */


#endif /* RTE_COMPONENTS_H */
