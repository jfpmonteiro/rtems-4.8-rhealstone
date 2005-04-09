/*-------------------------------------------------------------------------+
| This file contains the ARM BSP startup package. It includes application,
| board, and monitor specific initialization and configuration. The generic CPU
| dependent initialization has been performed before this routine is invoked. 
+--------------------------------------------------------------------------+
|
| Copyright (c) 2000 Canon Research Centre France SA.
| Emmanuel Raguet, mailto:raguet@crf.canon.fr
|
|   The license and distribution terms for this file may be
|   found in found in the file LICENSE in this distribution or at
|   http://www.rtems.com/license/LICENSE.
|
+--------------------------------------------------------------------------*/


#include <bsp.h>
#include <rtems/libcsupport.h>
#include <rtems/libio.h>
#include <s3c2400.h>

/*-------------------------------------------------------------------------+
| Global Variables
+--------------------------------------------------------------------------*/

extern void            *_sdram_size;
extern void            *_sdram_base;
extern void            *_bss_free_start;

unsigned long           free_mem_start;
unsigned long           free_mem_end;

/* The original BSP configuration table from the application and our copy of it
   with some changes. */

extern rtems_configuration_table  Configuration;
       rtems_configuration_table  BSP_Configuration;

rtems_cpu_table Cpu_table;                     /* CPU configuration table.    */
char            *rtems_progname = "RTEMS";               /* Program name - from main(). */

/*-------------------------------------------------------------------------+
| External Prototypes
+--------------------------------------------------------------------------*/
extern void rtems_irq_mngt_init(void);
void bsp_libc_init( void *, uint32_t, int );
void bsp_postdriver_hook(void);

/*-------------------------------------------------------------------------+
|         Function: bsp_pretasking_hook
|      Description: BSP pretasking hook.  Called just before drivers are
|                   initialized. Used to setup libc and install any BSP
|                   extensions. NOTE: Must not use libc (to do io) from here,
|                   since drivers are not yet initialized.
| Global Variables: None.
|        Arguments: None.
|          Returns: Nothing. 
+--------------------------------------------------------------------------*/
void bsp_pretasking_hook(void)
{
    uint32_t heap_start;
    uint32_t heap_size;

    /* 
     * Set up the heap. 
     */
    heap_start =  free_mem_start;
    heap_size = free_mem_end - free_mem_start;

    /* call rtems lib init - malloc stuff */
    bsp_libc_init((void *)heap_start, heap_size, 0);

#ifdef RTEMS_DEBUG

    rtems_debug_enable(RTEMS_DEBUG_ALL_MASK);

#endif /* RTEMS_DEBUG */

} /* bsp_pretasking_hook */
 
/*-------------------------------------------------------------------------+
|         Function: bsp_start
|      Description: Called before main is invoked.
| Global Variables: None.
|        Arguments: None.
|          Returns: Nothing. 
+--------------------------------------------------------------------------*/
void bsp_start_default( void )
{
    uint32_t cr;
    /* If we don't have command line arguments set default program name. */

    Cpu_table.pretasking_hook      = bsp_pretasking_hook; /* init libc, etc. */
    Cpu_table.predriver_hook       = NULL;                /* use system's    */
    Cpu_table.postdriver_hook      = bsp_postdriver_hook;
    Cpu_table.idle_task            = NULL;
                                          
    Cpu_table.do_zero_of_workspace = TRUE;
    Cpu_table.interrupt_stack_size = 4096;
    Cpu_table.extra_mpci_receive_server_stack = 0;

    /* stop all timers */
    rTCON=0x0;

    /* disable interrupts */
    rINTMOD=0x0;
    rINTMSK=BIT_ALLMSK; /* unmasked by drivers */
    rSRCPND=BIT_ALLMSK;
    rINTMSK=BIT_ALLMSK;
    rINTPND=BIT_ALLMSK;

    /* setup clocks 133/66/33) */
    rCLKDIVN=3;
    rMPLLCON=((M_MDIV<<12)+(M_PDIV<<4)+M_SDIV);

    /* set prescaler for timers 2,3,4 to 16(15+1) */
    cr=rTCFG0 & 0xFFFF00FF;
    rTCFG0=(cr | (15<<8));

    /* set prescaler for timers 0,1 to 1(0+1) */
    cr=rTCFG0 & 0xFFFFFF00;
    rTCFG0=(cr | (0<<0));

    /* Place RTEMS workspace at beginning of free memory. */
    BSP_Configuration.work_space_start = (void *)&_bss_free_start;

    free_mem_start = ((uint32_t)&_bss_free_start + BSP_Configuration.work_space_size);
    
    free_mem_end = ((uint32_t)&_sdram_base + (uint32_t)&_sdram_size);

    /*
     * Init rtems exceptions management
     */
    rtems_exception_init_mngt();

    /*
     * Init rtems interrupt management
     */
    rtems_irq_mngt_init();
    /*
     *  The following information is very useful when debugging.
     */

#if 0
    printk( "work_space_size = 0x%x\n", BSP_Configuration.work_space_size );
    printk( "maximum_extensions = 0x%x\n", BSP_Configuration.maximum_extensions );
    printk( "microseconds_per_tick = 0x%x\n",
            BSP_Configuration.microseconds_per_tick );
    printk( "ticks_per_timeslice = 0x%x\n",
            BSP_Configuration.ticks_per_timeslice );
    printk( "maximum_devices = 0x%x\n", BSP_Configuration.maximum_devices );
    printk( "number_of_device_drivers = 0x%x\n",
            BSP_Configuration.number_of_device_drivers );
    printk( "Device_driver_table = 0x%x\n",
            BSP_Configuration.Device_driver_table );
    
    printk( "_heap_size = 0x%x\n", _heap_size );
    /*  printk( "_stack_size = 0x%x\n", _stack_size );*/
    printk( "rtemsFreeMemStart = 0x%x\n", rtemsFreeMemStart );
    printk( "work_space_start = 0x%x\n", BSP_Configuration.work_space_start );
    printk( "work_space_size = 0x%x\n", BSP_Configuration.work_space_size );
#endif

}

/*
 *  By making this a weak alias for bsp_start_default, a brave soul
 *  can override the actual bsp_start routine used.
 */

void bsp_start (void) __attribute__ ((weak, alias("bsp_start_default")));

void bsp_reset(void)
{
    rtems_interrupt_level level;
    _CPU_ISR_Disable(level);
    printk("bsp_reset.....\n");
    while(1);
}

void LCD_BREAK()
{ 
    int x,y; 
    unsigned short color=0; 
    volatile unsigned char* framebuffer = (unsigned char*) 0x0C7B4000; 
    while(1) { 
        for(y = 0; y < 240; y++) {
            for(x = 0; x < 320; x++) {
                *(framebuffer + (239 - y) + (240 * x)) = color; 
            }
        }
        color++; 
    } 
}
