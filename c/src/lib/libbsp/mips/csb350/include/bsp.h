/*  bsp.h
 *
 *  This include file contains some definitions specific to the
 *  Cogent CSB350 Board.
 *
 *  COPYRIGHT (c) 1989-2000.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id$
 */

#ifndef __BSP_H__
#define __BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <bspopts.h>

#include <rtems.h>
#include <rtems/iosupp.h>
#include <rtems/console.h>
#include <rtems/clockdrv.h>
#include <libcpu/au1x00.h>

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
 *  Define the interrupt mechanism for Time Test 27
 */
int assert_sw_irw(unsigned32 irqnum);
int negate_sw_irw(unsigned32 irqnum);

#define MUST_WAIT_FOR_INTERRUPT 0

#define Install_tm27_vector( handler ) \
   (void) set_vector(handler, AU1X00_IRQ_SW0, 1);

#define Cause_tm27_intr() \
  do { \
     assert_sw_irq(0); \
  } while(0)

#define Clear_tm27_intr() \
  do { \
     negate_sw_irq(0); \
  } while(0)

#if 0
#define Lower_tm27_intr() \
  mips_enable_in_interrupt_mask( 0xff01 );
#else
#define Lower_tm27_intr() \
  do { \
     continue;\
  } while(0)
#endif

/* Constants */

/* miscellaneous stuff assumed to exist */

extern rtems_configuration_table BSP_Configuration;

/*
 *  Device Driver Table Entries
 */

/*
 * NOTE: Use the standard Console driver entry
 */
 
/*
 * NOTE: Use the standard Clock driver entry
 */

/*
 * Network driver configuration
 */
extern struct rtems_bsdnet_ifconfig *config;

int rtems_au1x00_emac_attach(struct rtems_bsdnet_ifconfig *config, 
                             int attaching);
#define RTEMS_BSP_NETWORK_DRIVER_NAME	"eth0"
#define RTEMS_BSP_NETWORK_DRIVER_ATTACH	rtems_au1x00_emac_attach

/* functions */

void bsp_cleanup( void );

rtems_isr_entry set_vector(
  rtems_isr_entry, rtems_vector_number, int );

#ifdef __cplusplus
}
#endif

#endif
/* end of include file */