/*
 *  Instantiate the clock driver shell.
 *
 *  This usese the TOY (Time of Year) timer to implement the clock.
 *
 *  Copyright (c) 2005 by Cogent Computer Systems
 *  Written by Jay Monkman <jtm@lopingdog.com>
 *	
 *  The license and distribution terms for this file may be
 *  found in found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id$
 */

#include <rtems.h>
#include <bsp.h>
#include <libcpu/au1x00.h>


unsigned32 tick_interval;
unsigned32 last_match;

#define CLOCK_VECTOR AU1X00_IRQ_TOY_MATCH2

#define Clock_driver_support_at_tick()                        \
  do {                                                        \
    while (AU1X00_SYS_CNTCTRL(AU1X00_SYS_ADDR) & AU1X00_SYS_CNTCTRL_TM0); \
    last_match = AU1X00_SYS_TOYREAD(AU1X00_SYS_ADDR); \
    AU1X00_SYS_TOYMATCH2(AU1X00_SYS_ADDR) = last_match + tick_interval; \
    au_sync(); \
  } while(0)

/* Set for rising edge interrupt */
#define Clock_driver_support_install_isr( _new, _old )  \
  do {                                                  \
        _old = set_vector( _new, AU1X00_IRQ_TOY_MATCH2, 1 );     \
        AU1X00_IC_MASKCLR(AU1X00_IC0_ADDR) = AU1X00_IC_IRQ_TOY_MATCH2; \
        AU1X00_IC_SRCSET(AU1X00_IC0_ADDR) = AU1X00_IC_IRQ_TOY_MATCH2; \
        AU1X00_IC_CFG0SET(AU1X00_IC0_ADDR) = AU1X00_IC_IRQ_TOY_MATCH2; \
        AU1X00_IC_CFG1CLR(AU1X00_IC0_ADDR) = AU1X00_IC_IRQ_TOY_MATCH2; \
        AU1X00_IC_CFG2CLR(AU1X00_IC0_ADDR) = AU1X00_IC_IRQ_TOY_MATCH2; \
        AU1X00_IC_ASSIGNSET(AU1X00_IC0_ADDR) = AU1X00_IC_IRQ_TOY_MATCH2; \
  } while(0)

void au1x00_clock_init(void)
{
    unsigned32 wakemask;
    /* Clear the trim register */ 
    AU1X00_SYS_TOYTRIM(AU1X00_SYS_ADDR) = 0; 
    
    /* Clear the TOY counter */ 
    while (AU1X00_SYS_CNTCTRL(AU1X00_SYS_ADDR) & AU1X00_SYS_CNTCTRL_TS); 
    AU1X00_SYS_TOYWRITE(AU1X00_SYS_ADDR) = 0; 
    while (AU1X00_SYS_CNTCTRL(AU1X00_SYS_ADDR) & AU1X00_SYS_CNTCTRL_TS); 
    
    wakemask = AU1X00_SYS_WAKEMSK(AU1X00_SYS_ADDR); 
    wakemask |= AU1X00_SYS_WAKEMSK_M20;
    AU1X00_SYS_WAKEMSK(AU1X00_SYS_ADDR) = wakemask; 
    AU1X00_IC_WAKESET(AU1X00_IC0_ADDR) = AU1X00_IC_IRQ_TOY_MATCH2;
    
    tick_interval = 32768 * rtems_configuration_get_microseconds_per_tick(); 
    tick_interval = tick_interval / 1000000; 
    printk("tick_interval = %d\n", tick_interval); 
    
    last_match = AU1X00_SYS_TOYREAD(AU1X00_SYS_ADDR); 
    AU1X00_SYS_TOYMATCH2(AU1X00_SYS_ADDR) = last_match + (50*tick_interval); 
    AU1X00_IC_MASKSET(AU1X00_IC0_ADDR) = AU1X00_IC_IRQ_TOY_MATCH2;
    while (AU1X00_SYS_CNTCTRL(AU1X00_SYS_ADDR) & AU1X00_SYS_CNTCTRL_TM0); 
}

#define Clock_driver_support_initialize_hardware() \
  do {                                                  \
     au1x00_clock_init(); \
  } while(0)

                  

#define Clock_driver_support_shutdown_hardware()

#include "../../../shared/clockdrv_shell.c"