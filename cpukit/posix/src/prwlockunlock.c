/*
 *  POSIX RWLock Manager -- Release a lock held on a RWLock Instance
 *
 *  COPYRIGHT (c) 1989-2006.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <errno.h>

#include <rtems/system.h>
#include <rtems/posix/rwlock.h>

/*
 *  pthread_rwlock_unlock
 *
 *  This directive attempts to release a lock on an RWLock.
 *
 *  Input parameters:
 *    rwlock          - pointer to rwlock id
 *
 *  Output parameters:
 *    0          - if successful
 *    error code - if unsuccessful
 */

int pthread_rwlock_unlock(
  pthread_rwlock_t  *rwlock
)
{
  POSIX_RWLock_Control  *the_rwlock;
  Objects_Locations      location;
  
  if ( !rwlock )
    return EINVAL;

  the_rwlock = _POSIX_RWLock_Get( rwlock, &location );
  switch ( location ) {

    case OBJECTS_REMOTE:
    case OBJECTS_ERROR:
      return EINVAL;

    case OBJECTS_LOCAL:

      /* XXX */

      _Thread_Enable_dispatch();
      return 0;
  }

  return POSIX_BOTTOM_REACHED();
}