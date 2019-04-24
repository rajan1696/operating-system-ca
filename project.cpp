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

void
sema_up (struct semaphore *sema) 
{
  enum intr_level old_level;
  bool yield = false;

  ASSERT (sema != NULL);

  old_level = intr_disable ();
  if (!list_empty (&sema->waiters)) 
  {
    struct thread *t = list_entry (list_pop_front (&sema->waiters),
                        struct thread, elem);
    thread_unblock (t);

    /* Yield to the newly unblocked thread if it has higher priority. */
    if (t->priority > thread_current ()->priority)
      yield = true;
  }
  sema->value++;
  intr_set_level (old_level);

  if (yield)
  {
    if (!intr_context ())
      thread_yield ();
    else
      intr_yield_on_return ();
  }
}

static void sema_test_helper (void *sema_);

void
sema_self_test (void) 
{
  struct semaphore sema[2];
  int i;

  printf ("Testing semaphores...");
  sema_init (&sema[0], 0);
  sema_init (&sema[1], 0);
  thread_create ("sema-test", PRI_DEFAULT, sema_test_helper, &sema);
  for (i = 0; i < 10; i++) 
    {
      sema_up (&sema[0]);
      sema_down (&sema[1]);
    }
  printf ("done.\n");
}

