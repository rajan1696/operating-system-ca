#include <threads/synch.h>
#include <stdio.h>
#include <string.h>
#include <threads/interrupt.h>
#include <threads/thread.h>

void
sema_init (struct semaphore *sema, unsigned value) 
{
  ASSERT (sema != NULL);

  sema->value = value;
  list_init (&sema->waiters);
}


void
sema_down (struct semaphore *sema) 
{
  enum intr_level old_level;

  ASSERT (sema != NULL);
  ASSERT (!intr_context ());

  old_level = intr_disable ();
  while (sema->value == 0) 
    {
      /* Keep the waiting list sorted, highest priority first. */
      list_insert_ordered (&sema->waiters, &thread_current ()->elem,
        more_prio, NULL);
      thread_block ();
    }
  sema->value--;
  intr_set_level (old_level);
}

bool
sema_try_down (struct semaphore *sema) 
{
  enum intr_level old_level;
  bool success;

  ASSERT (sema != NULL);

  old_level = intr_disable ();
  if (sema->value > 0) 
    {
      sema->value--;
      success = true; 
    }
  else
    success = false;
  intr_set_level (old_level);

  return success;
}

