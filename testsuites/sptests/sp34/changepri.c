/*
 *  Test program to demonstrate reordering of threads on thread queues
 *  when their priority changes.
 *
 *  $Id$
 */

#include <bsp.h>
#include <stdio.h>

/********************************************************************/
/* define this to use the RTEMS 4.5 scheme for object names */
#define TEST_ON_RTEMS_45

/* define this to print the Id of the calling task */
/* #define TEST_ON_TASK_ID */

/********************************************************************/

#include <bsp.h>
#include <stdio.h>
#include "tmacros.h"

rtems_task BlockingTasks(rtems_task_argument arg);

/*
 *  CallerName -- print the calling tasks name or id as configured
 */
const char *CallerName()
{
  static char buffer[32];
#if defined(TEST_PRINT_TASK_ID)
  sprintf( buffer, "0x%08x -- %d",
      _Thread_Executing->Object.id, _Thread_Executing->current_priority );
#else
  union {
    uint32_t u;
    unsigned char c[4];
  } TempName;

  #if defined(TEST_ON_RTEMS_45)
    TempName.u = *(uint32_t *)_Thread_Executing->Object.name;
  #else
    TempName.u = _Thread_Executing->Object.name;
  #endif
  sprintf( buffer, "%c%c%c%c -- %d",
      TempName.c[0], TempName.c[1], TempName.c[2], TempName.c[3],
      _Thread_Executing->current_priority
  );
#endif
  return buffer;
}

#define NUMBER_OF_BLOCKING_TASKS 5

/* RTEMS ids of blocking threads */
rtems_id  Blockers[NUMBER_OF_BLOCKING_TASKS];

/* Semaphore they are all blocked on */
rtems_id  Semaphore;

rtems_task BlockingTasks(rtems_task_argument arg)
{
  rtems_status_code   status;
  rtems_task_priority opri;
  rtems_task_priority npri;

  status = rtems_task_set_priority(RTEMS_SELF, RTEMS_CURRENT_PRIORITY, &opri);
  directive_failed( status, "rtems_task_set_priority" );

  printf("semaphore_obtain -- BlockingTask %d @ pri=%d) blocks\n", arg, opri);
  status = rtems_semaphore_obtain(Semaphore, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  directive_failed( status, "rtems_semaphore_obtain" );

  /* priority should have changed while blocked */
  status = rtems_task_set_priority(RTEMS_SELF, RTEMS_CURRENT_PRIORITY, &npri);
  directive_failed( status, "rtems_task_set_priority" );

  printf("semaphore_obtain -- BlockingTask %d @ pri=%d) returns\n", arg, npri);

  (void) rtems_task_delete( RTEMS_SELF );
}

/*************************************************************************/
/**********************        INITIALIZATION        *********************/
/*************************************************************************/

rtems_task Init(rtems_task_argument ignored)
{
  rtems_status_code   status;   
  int                 i;
  
  puts( "\n\n*** TEST 34 ***" );

  /* Create synchronisation semaphore for LocalHwIsr -> Test Tasks */
  status = rtems_semaphore_create(
    rtems_build_name ('S', 'E', 'M', '1'),           /* name */
    0,                                               /* initial count = 0 */
    RTEMS_LOCAL                   | 
    RTEMS_COUNTING_SEMAPHORE      | 
    RTEMS_PRIORITY,
    0,
    &Semaphore);                                    /* *id */
  directive_failed( status, "rtems_semaphore_create" );
  
  /* Create and start all tasks in the test */

  for (i = 0; i < NUMBER_OF_BLOCKING_TASKS; i++) {
    status = rtems_task_create(
      rtems_build_name('B','L','K','0'+i),               /* Name */
      2+i,                                               /* Priority */
      RTEMS_MINIMUM_STACK_SIZE*2,                        /* Stack size (8KB) */
      RTEMS_DEFAULT_MODES | RTEMS_NO_ASR,                /* Mode */
      RTEMS_DEFAULT_ATTRIBUTES | RTEMS_FLOATING_POINT,   /* Attributes */
      &Blockers[i]);                                     /* Assigned ID */
    directive_failed( status, "rtems_task_create (BLKn)" );
  
    printf( "Blockers[%d] Id = 0x%08x\n", i, Blockers[i] );
    status = rtems_task_start(Blockers[i], BlockingTasks, i);
    directive_failed( status, "rtems_task_start (BLKn)" );
  }

  status = rtems_task_wake_after( 100 );
  directive_failed( status, "rtems_task_wake_after" );

  puts( "rtems_task_set_priority -- invert priorities of tasks" );
  for (i = 0; i < NUMBER_OF_BLOCKING_TASKS; i++) {
    rtems_task_priority opri;
    rtems_task_priority npri= 2 + NUMBER_OF_BLOCKING_TASKS - i - 1;

    status = rtems_task_set_priority(Blockers[i], npri, &opri);
    directive_failed( status, "rtems_task_set_priority" );
  }

  for (i = 0; i < NUMBER_OF_BLOCKING_TASKS; i++) {
    puts( "rtems_semaphore_release -- OK" );
    status = rtems_semaphore_release(Semaphore);
    directive_failed( status, "rtems_semaphore_release" );

    status = rtems_task_wake_after( 100 );
    directive_failed( status, "rtems_task_wake_after" );
  }

  /* exit the test */
  puts( "*** END OF TEST 34 ***" );
  exit(0);
}

/* configuration information */

#define CONFIGURE_TEST_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_TEST_NEEDS_CLOCK_DRIVER

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_MEMORY_OVERHEAD 64

#define CONFIGURE_MAXIMUM_TASKS 6
#define CONFIGURE_MAXIMUM_SEMAPHORES 1

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

/* end of file */