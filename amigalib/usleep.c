/* This code is taken straight from the example in manual */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <devices/timer.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>

/* Our timer sub-routines */
void delete_timer(struct timerequest *);
void wait_for_timer(struct timerequest *, struct timeval *);
struct timerequest *create_timer(ULONG);

void usleep(unsigned int);

struct Library *TimerBase;	/* to get at the time comparison functions */

struct timerequest *
create_timer(ULONG unit)
{
  /* return a pointer to a timer request.  If any problem, return NULL */
  LONG error;
  struct MsgPort *timerport;
  struct timerequest *TimerIO;

  timerport = CreatePort(0, 0);
  if (timerport == NULL) {
    return (NULL);
  }
  TimerIO = (struct timerequest *)
    CreateExtIO(timerport, sizeof(struct timerequest));
  if (TimerIO == NULL) {
    DeletePort(timerport);	/* Delete message port */
    return (NULL);
  }
  error = OpenDevice(TIMERNAME, unit, (struct IORequest *) TimerIO, 0L);
  if (error != 0) {
    delete_timer(TimerIO);
    return (NULL);
  }
  return (TimerIO);
}

/* more precise timer than AmigaDOS Delay() */
void
usleep(unsigned int tim)
{
  struct timeval tv;
  struct timerequest *tr;

  tv.tv_secs = tim / 1000000;
  tv.tv_micro = tim % 1000000;
  /* get a pointer to an initialized timer request block */
  tr = create_timer(UNIT_VBLANK);

  /* any nonzero return says usleep routine didn't work. */
  if (tr == NULL) {
    return;
  }
  wait_for_timer(tr, &tv);

  /* deallocate temporary structures */
  delete_timer(tr);
  return;
}

void
wait_for_timer(struct timerequest *tr, struct timeval *tv)
{
  tr->tr_node.io_Command = TR_ADDREQUEST;	/* add a new timer request */

  /* structure assignment */
  tr->tr_time = *tv;

  /* post request to the timer -- will go to sleep till done */
  DoIO((struct IORequest *) tr);
}

void
delete_timer(struct timerequest *tr)
{
  struct MsgPort *tp;
  if (tr != 0) {
    tp = tr->tr_node.io_Message.mn_ReplyPort;
    if (tp != 0) {
      DeletePort(tp);
    }
    CloseDevice((struct IORequest *) tr);
    DeleteExtIO((struct IORequest *) tr);
  }
}
