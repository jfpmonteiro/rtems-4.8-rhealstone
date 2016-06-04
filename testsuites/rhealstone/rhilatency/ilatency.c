/* Copyright 2014 Daniel Ramirez (javamonn@gmail.com)
 *
 * This file's license is 2-clause BSD as in this distribution's LICENSE.2 file.
 */

/*
 *  WARNING!!!!!!!!!
 *
 *  THIS TEST USES INTERNAL RTEMS VARIABLES!!!
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <timesys.h>
#include <btimer.h>
#include <coverhd.h>
#include <bsp.h>

#define _RTEMS_TMTEST27
#include <tm27.h>

const char rtems_test_name[] = "RHILATENCY";

#define BENCHMARKS 50000

rtems_task Task_1(
  rtems_task_argument argument
);

uint32_t   Interrupt_nest;
uint32_t   timer_overhead;
uint32_t   Interrupt_enter_time;

rtems_isr Isr_handler(
  rtems_vector_number vector
);

rtems_task Init(
  rtems_task_argument argument
)
{
  rtems_status_code status;
  rtems_id Task_id;

  Print_Warning();

  /* TEST_BEGIN(); */
  printf("*** BEGIN OF TEST RHTASKSWITCH ***\n");

#define LOW_PRIORITY (RTEMS_MAXIMUM_PRIORITY - 1u)
  status = rtems_task_create(
    rtems_build_name( 'T', 'A', '1', ' ' ),
    LOW_PRIORITY,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &Task_id
  );
  directive_failed( status, "rtems_task_create Task_1" );

  status = rtems_task_start( Task_id, Task_1, 0 );
  directive_failed( status, "rtems_task_start Task_1" );

  Timer_initialize();
  Timer_read();
  Timer_initialize();
  timer_overhead = Timer_read();

  status = rtems_task_delete( RTEMS_SELF );
  directive_failed( status, "rtems_task_delete of RTEMS_SELF" );
}

rtems_task Task_1(
  rtems_task_argument argument
)
{
  Install_tm27_vector( Isr_handler ) ;
  Interrupt_nest = 0;

  /* Benchmark code */
  Timer_initialize();
  /* goes to Isr_handler */
  Cause_tm27_intr();

  put_time(
    "Rhealstone: Interrupt Latency",
    Interrupt_enter_time,
    1,                             /* Only Rhealstone that isn't an average */
    timer_overhead,
    0
  );

  /* TEST_END(); */
  printf("*** END OF TEST RHTASKSWITCH ***\n");
  rtems_test_exit( 0 );
}

rtems_isr Isr_handler(
  rtems_vector_number vector
)
{
  /* See how long it took system to recognize interrupt */
  Interrupt_enter_time = Timer_read();
  Clear_tm27_intr();
}

/* configuration information */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_TIMER_DRIVER
#define CONFIGURE_MAXIMUM_TASKS              2
#define CONFIGURE_TICKS_PER_TIMESLICE        0
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_SCHEDULER_PRIORITY

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
