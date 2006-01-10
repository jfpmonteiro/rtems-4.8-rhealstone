/*===============================================================*\
| Project: RTEMS generic MPC5200 BSP                              |
+-----------------------------------------------------------------+
| Partially based on the code references which are named below.   |
| Adaptions, modifications, enhancements and any recent parts of  |
| the code are:                                                   |
|                    Copyright (c) 2005                           |
|                    Embedded Brains GmbH                         |
|                    Obere Lagerstr. 30                           |
|                    D-82178 Puchheim                             |
|                    Germany                                      |
|                    rtems@embedded-brains.de                     |
+-----------------------------------------------------------------+
| The license and distribution terms for this file may be         |
| found in the file LICENSE in this distribution or at            |
|                                                                 |
| http://www.rtems.com/license/LICENSE.                           |
|                                                                 |
+-----------------------------------------------------------------+
| this file contains board specific definitions                   |
\*===============================================================*/

#ifndef __GEN5200_BSP_h
#define __GEN5200_BSP_h

/*
 * distinguish board characteristics
 */
/*
 * for PM520 mdule on a ZE30 carrier
 */
#if defined(PM520_ZE30)
#define PM520
#define GPIOPCR_INITMASK 0x337F3F77
#define GPIOPCR_INITVAL  0x01552114
/* we have PSC1/4/5/6 */
/* #define GEN5200_UART_AVAIL_MASK 0x39 */
#define GEN5200_UART_AVAIL_MASK 0x39
#endif
/*
 * for PM520 mdule on a CR825 carrier
 */
#if defined(PM520_CR825)
#define PM520
#define GPIOPCR_INITMASK 0x330F0F77
#define GPIOPCR_INITVAL  0x01050444
/* we have PSC1/2/3*/
#define GEN5200_UART_AVAIL_MASK 0x07
#endif

#if defined(BRS5L)
/*
 * IMD Custom Board BRS5L
 */
#define GPIOPCR_INITMASK 0xb30F0F77
#define GPIOPCR_INITVAL  0x91050444
/* we have PSC1/2/3 */
#define GEN5200_UART_AVAIL_MASK 0x07
/*
 * address range definitions
 */
/* ROM definitions (2 MB) */
#define	ROM_START	0xFFE00000
#define	ROM_END		0xFFFFFFFF
#define	BOOT_START      ROM_START
#define	BOOT_END        ROM_END

/* SDRAM definitions (256 MB) */
#define	RAM_START    0x00000000
#define	RAM_END      0x0FFFFFFF

/* DPRAM definitions (64 KB) */
#define	DPRAM_START  0xFF000000
#define	DPRAM_END    0xFF0003FF

/* internal memory map definitions (64 KB) */
#define	MBAR         0xF0000000

/* we need the low level initialization in start.S*/
#define NEED_LOW_LEVEL_INIT

#define HAS_NVRAM_93CXX
#elif defined (PM520)

/* 
 * MicroSys PM520 internal memory map definitions
 */
#define	MBAR         0xF0000000
#define HAS_UBOOT

#else
#error "board type not defined"
#endif

#ifndef ASM

#ifdef __cplusplus
extern "C" {
#endif

#include "bspopts.h"

#include <rtems.h>
#include <rtems/console.h>
#include <rtems/clockdrv.h>
#include <i2cdrv.h>

#if defined(HAS_UBOOT)
#define CONFIG_MPC5xxx
#include <u-boot.h>
extern bd_t *uboot_bdinfo_ptr;
extern bd_t uboot_bdinfo_copy;
#endif

/*
 * Network driver configuration
 */
struct rtems_bsdnet_ifconfig;
extern int rtems_mpc5200_fec_driver_attach_detach (struct rtems_bsdnet_ifconfig *config, int attaching);
#define RTEMS_BSP_NETWORK_DRIVER_NAME	"eth1"
#define RTEMS_BSP_NETWORK_DRIVER_ATTACH	rtems_mpc5200_fec_driver_attach_detach

/* miscellaneous stuff assumed to exist */

extern rtems_configuration_table BSP_Configuration;
/*
 * We need to decide how much memory will be non-cacheable. This
 * will mainly be memory that will be used in DMA (network and serial
 * buffers).
 */
/*
#define NOCACHE_MEM_SIZE 512*1024
*/

/*
 *  Define the time limits for RTEMS Test Suite test durations.
 *  Long test and short test duration limits are provided.  These
 *  values are in seconds and need to be converted to ticks for the
 *  application.
 *
 */

#define MAX_LONG_TEST_DURATION       300 /* 5 minutes = 300 seconds */
#define MAX_SHORT_TEST_DURATION      3   /* 3 seconds */

/*
 *  Stuff for Time Test 27
 */
#define MUST_WAIT_FOR_INTERRUPT 0

/*
 *  Device Driver Table Entries
 */

/*
 * NOTE: Use the standard Console driver entry
 */

/*
 * NOTE: Use the standard Clock driver entry
 */

#ifdef HAS_NVRAM_93CXX
#define NVRAM_DRIVER_TABLE_ENTRY \
  { nvram_driver_initialize, nvram_driver_open, nvram_driver_close, \
    nvram_driver_read, nvram_driver_write, NULL }
#endif

#define RTC_DRIVER_TABLE_ENTRY \
    { rtc_initialize, NULL, NULL, NULL, NULL, NULL }
extern rtems_device_driver rtc_initialize(
    rtems_device_major_number major,
    rtems_device_minor_number minor,
    void *arg
);

/*
 * indicate, that BSP has IDE driver
 */
#define RTEMS_BSP_HAS_IDE_DRIVER

/*
 * How many libio files we want
 */
#define BSP_LIBIO_MAX_FDS       20

/* functions */

void bsp_cleanup(void);

/* console modes (only termios) */
#ifdef  PRINTK_MINOR
#undef  PRINTK_MINOR
#endif
#define PRINTK_MINOR PSC1_MINOR

#define SINGLE_CHAR_MODE
#define UARTS_USE_TERMIOS_INT   1

/* ata modes */
#undef ATA_USE_INT

/* clock settings */
#if defined(HAS_UBOOT)
#define IPB_CLOCK (uboot_bdinfo_ptr->bi_ipbfreq)
#define XLB_CLOCK (uboot_bdinfo_ptr->bi_busfreq)
#define G2_CLOCK  (uboot_bdinfo_ptr->bi_intfreq)
#else
#define IPB_CLOCK 33000000   /* 33 MHz */
#define XLB_CLOCK 66000000   /* 66 MHz */
#define G2_CLOCK  231000000  /* 231 MHz */
#endif

/* slicetimer settings */
#define USE_SLICETIMER_0     TRUE
#define USE_SLICETIMER_1     FALSE

#ifdef __cplusplus
}
#endif

#endif /* ASM */

#endif /* GEN5200 */
/* end of include file */