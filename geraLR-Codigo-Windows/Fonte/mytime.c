/*
*-----------------------------------------------------------------------
*
*   File         : time.c
*   Created      : 1994-05-19
*   Last Modified: 2012-05-28 (2018-03-28: realTimeStrSize bug fixed)
*
*   Time-related functions and stopwatches
*
*-----------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"
#include "mytime.h"

/*
*---------------------------------------------------------------------
*
*   INTERFACE (visible from other modules)
*
*---------------------------------------------------------------------
*/

/*                     */
/* Function prototypes */
/*                     */

void              TIME_init_stopwatches    (void);
TIME_t_stopwatch  TIME_setup_stopwatch     (void);
TIME_t_seconds    TIME_read_stopwatch      (TIME_t_stopwatch index);
TIME_t_seconds    TIME_stop_stopwatch      (TIME_t_stopwatch index);
TIME_t_seconds    TIME_start_stopwatch     (TIME_t_stopwatch index);
TIME_t_seconds    TIME_clear_stopwatch     (TIME_t_stopwatch index);
TIME_t_seconds    TIME_reset_stopwatch     (TIME_t_stopwatch index);
TIME_t_seconds    TIME_dismiss_stopwatch   (TIME_t_stopwatch index);
char             *TIME_startTime_stopwatch (TIME_t_stopwatch index);
char             *TIME_getCurrentTimeStr   (void);
char             *TIME_time2realTimeStr    (time_t real_time);
char             *TIME_secs2elapsedTimeStr (TIME_t_seconds elapsed_time);
void              TIME_time_delay          (TIME_t_seconds seconds);

/* To prevent "implicit declaration" warnings */

int snprintf (char *str, size_t size, const char *format, ...);

/*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION
*
*---------------------------------------------------------------------
*/

/*                         */
/* Definition of constants */
/*                         */

/* Precision of clock() is 1/100 sec */

#ifdef CLOCKS_PER_SEC
#  define CLOCKS_PER_SECOND  (CLOCKS_PER_SEC)
#else
#  ifdef CLK_TCK
#    define CLOCKS_PER_SECOND  (CLK_TCK)
#  else
#    define CLOCKS_PER_SECOND  (1000000)
#  endif
#endif

/* After these secs clock() wraps around */

#define CLOCK_CYCLE   ((max_LONG)/CLOCKS_PER_SECOND)

#define maxStopWatches       30
#define realTimeStrSize      30
#define elapsedTimeStrSize   19
#define elapsedTimeFormat   "%2dd %2dh %2dm %5.2fs"

/*                               */
/* Data structures and variables */
/*                               */

typedef char
  t_realTimeStr [realTimeStrSize];

typedef char
  t_elapsedTimeStr [elapsedTimeStrSize];

typedef struct {
  bool
    b_in_use,
    b_running;
  clock_t
    initial_clock;
  time_t
    initial_time;
  TIME_t_seconds
    curr_reading,
    acc_reading;
  t_realTimeStr
    starting_timeStr;
}
  t_stopWatch;

static t_stopWatch
  stopWatch [maxStopWatches],
  dummy_stopWatch;

static bool
  b_init_OK = false;

/*                     */
/* Function prototypes */
/*                     */

static bool valid_stopWatch  (TIME_t_stopwatch index);
static void update_stopWatch (TIME_t_stopwatch index);

/*
*---------------------------------------------------------------------
*   Initializes stopwatches
*---------------------------------------------------------------------
*/

void TIME_init_stopwatches (void)
{
 int
   c_watch;

 dummy_stopWatch.b_in_use      = false;
 dummy_stopWatch.b_running     = false;
 dummy_stopWatch.initial_clock = (clock_t) 0;
 dummy_stopWatch.initial_time  = (time_t)  0;
 dummy_stopWatch.curr_reading  = (TIME_t_seconds)  0;
 dummy_stopWatch.acc_reading   = (TIME_t_seconds)  0;
 strcpy (dummy_stopWatch.starting_timeStr, "");
 for (c_watch = 0; c_watch < maxStopWatches; c_watch++)
   stopWatch [c_watch] = dummy_stopWatch;
 b_init_OK = true;
}

/*
*---------------------------------------------------------------------
*   Sets up a stopwatch without starting it, and return its number
*---------------------------------------------------------------------
*/

TIME_t_stopwatch TIME_setup_stopwatch (void)
{
 TIME_t_stopwatch
   index = 0;
 t_stopWatch
   *p_stopWatch = stopWatch;

 if (! b_init_OK) {
   printf ("Must call init_StopWatches() first.\n");
   return (-1);
 }
 while ((p_stopWatch->b_in_use) && (index < maxStopWatches)) {
   index++;
   p_stopWatch++;
 }
 if (index < maxStopWatches) {
   p_stopWatch->b_in_use      = true;
   p_stopWatch->b_running     = false;
   p_stopWatch->initial_clock = (clock_t) 0;
   p_stopWatch->initial_time  = (time_t)  0;
   p_stopWatch->curr_reading  = (TIME_t_seconds)  0;
   p_stopWatch->acc_reading   = (TIME_t_seconds)  0;
   strcpy (p_stopWatch->starting_timeStr, "");
   return (index);
 }
 else {
   printf ("Sorry, all %d stopwatches in use.\n", (int) maxStopWatches);
   return (-1);
 }
}

/*
*---------------------------------------------------------------------
*   Checks whether a given stopwatch exists and is active
*---------------------------------------------------------------------
*/

static bool valid_stopWatch (TIME_t_stopwatch index)
{
 if (! b_init_OK) {
   printf ("Must call init_StopWatches() first.\n");
   return (false);
 }
 return ((index >= 0) &&
	 (index < maxStopWatches) &&
	 (stopWatch[index].b_in_use));
}

/*
*---------------------------------------------------------------------
*   Updates the "running_secs" field of a stopwatch
*---------------------------------------------------------------------
*/

static void update_stopWatch (TIME_t_stopwatch index)
{
 TIME_t_seconds
   elapsed_time,
   elapsed_clock,
   elapsed_secs;
 t_stopWatch
   *p_stopWatch;

 /* No need to check if this is a valid index, as it has */
 /* already been done by whoever called this procedure   */

 p_stopWatch = (t_stopWatch *) stopWatch + index;
 if (! p_stopWatch->b_running)
   return;
 elapsed_time = (TIME_t_seconds)
   (time(NULL) - p_stopWatch->initial_time);
 if (elapsed_time >= (TIME_t_seconds) CLOCK_CYCLE)
   elapsed_secs = elapsed_time;
 else {
   elapsed_clock = (TIME_t_seconds)
     (clock() - p_stopWatch->initial_clock) / CLOCKS_PER_SECOND;
   if (elapsed_clock < (TIME_t_seconds) 0)
     elapsed_clock += (TIME_t_seconds) CLOCK_CYCLE;
   elapsed_secs = elapsed_clock;
 }
 p_stopWatch->curr_reading = elapsed_secs;
}

/*
*---------------------------------------------------------------------
*   Returns the current reading of a stopwatch
*---------------------------------------------------------------------
*/

TIME_t_seconds TIME_read_stopwatch (TIME_t_stopwatch index)
{
 TIME_t_seconds
   running_time;

 if (! valid_stopWatch (index))
   return (TIME_NULL_TIME);
 update_stopWatch (index);
 running_time =
   stopWatch[index].acc_reading +
   stopWatch[index].curr_reading;
 return (running_time);
}

/*
*---------------------------------------------------------------------
*   Stops (pauses) a stopwatch and returns its current reading
*---------------------------------------------------------------------
*/

TIME_t_seconds TIME_stop_stopwatch (TIME_t_stopwatch index)
{
 TIME_t_seconds
   running_secs;
 t_stopWatch
   *p_stopWatch;

 if (! valid_stopWatch (index))
   return (TIME_NULL_TIME);
 running_secs = TIME_read_stopwatch (index);
 p_stopWatch = (t_stopWatch *) stopWatch + index;
 p_stopWatch->acc_reading += p_stopWatch->curr_reading;
 p_stopWatch->curr_reading = (TIME_t_seconds) 0;
 stopWatch[index].b_running = false;
 return (running_secs);
}

/*
*---------------------------------------------------------------------
*   Restarts a stopwatch and return its current reading
*---------------------------------------------------------------------
*/

TIME_t_seconds TIME_start_stopwatch (TIME_t_stopwatch index)
{
 TIME_t_seconds
   running_secs;
 t_stopWatch
   *p_stopWatch;

 if (! valid_stopWatch (index))
   return (TIME_NULL_TIME);
 running_secs = TIME_read_stopwatch (index);
 p_stopWatch = (t_stopWatch *) stopWatch + index;
 p_stopWatch->initial_clock = clock();
 p_stopWatch->initial_time  = time(NULL);
 p_stopWatch->b_running     = true;
 strcpy (p_stopWatch->starting_timeStr, TIME_getCurrentTimeStr());
 return (running_secs);
}

/*
*---------------------------------------------------------------------
*   Clears a stopwatch's reading and return its reading prior to this
*   clearing. Does not change the stopwatch's running status.
*---------------------------------------------------------------------
*/

TIME_t_seconds TIME_clear_stopwatch (TIME_t_stopwatch index)
{
 TIME_t_seconds
   running_secs;
 t_stopWatch
   *p_stopWatch;

 if (! valid_stopWatch (index))
   return (TIME_NULL_TIME);
 running_secs = TIME_read_stopwatch (index);
 p_stopWatch = (t_stopWatch *) stopWatch + index;
 p_stopWatch->initial_clock = clock();
 p_stopWatch->initial_time  = time(NULL);
 p_stopWatch->acc_reading   = (TIME_t_seconds) 0;
 p_stopWatch->curr_reading  = (TIME_t_seconds) 0;
 return (running_secs);
}

/*
*---------------------------------------------------------------------
*   Stops a stopwatch, clears its reading and starts it again.
*   Returns its reading prior to this resetting.
*---------------------------------------------------------------------
*/

TIME_t_seconds TIME_reset_stopwatch (TIME_t_stopwatch index)
{
 TIME_t_seconds
   running_secs;

 if (! valid_stopWatch (index))
   return (TIME_NULL_TIME);
 running_secs = TIME_stop_stopwatch (index);
 (void) TIME_clear_stopwatch (index);
 (void) TIME_start_stopwatch (index);
 return (running_secs);
}

/*
*---------------------------------------------------------------------
*   Dismisses a stopwatch and returns its reading
*   prior to the dismissal
*---------------------------------------------------------------------
*/

TIME_t_seconds TIME_dismiss_stopwatch (TIME_t_stopwatch index)
{
 TIME_t_seconds
   running_secs;

 if (! valid_stopWatch (index))
   return (TIME_NULL_TIME);
 running_secs = TIME_read_stopwatch (index);
 stopWatch[index] = dummy_stopWatch;
 return (running_secs);
}

/*
*---------------------------------------------------------------------
*   Returns a pointer to a (static) string containg the time
*   when a given stopwatch was started
*---------------------------------------------------------------------
*/

char *TIME_startTime_stopwatch (TIME_t_stopwatch index)
{
 static t_realTimeStr
   timeStr;

 if (! valid_stopWatch (index))
   strcpy (timeStr, "Invalid stopwatch.");
 else
   strcpy (timeStr, stopWatch[index].starting_timeStr);
 return (timeStr);
}

/*
*---------------------------------------------------------------------
*   Returns a pointer to a static string containg the current time
*---------------------------------------------------------------------
*/

char *TIME_getCurrentTimeStr (void)
{
 time_t
   CurrTime = time(NULL);
 static t_realTimeStr
   timeStr;

 strcpy (timeStr, ctime (&CurrTime));
 (void) strtok (timeStr, "\n");
 return (timeStr);
}

/*
*---------------------------------------------------------------------
*   Converts a time_t into a full (static) string.
*   Returns a pointer to the string.
*---------------------------------------------------------------------
*/

char *TIME_time2realTimeStr (time_t real_time)
{
 static t_realTimeStr
   timeStr;

 strcpy (timeStr, ctime (&real_time));
 (void) strtok (timeStr, "\n");
 return (timeStr);
}

/*
*---------------------------------------------------------------------
*   Converts time in seconds into a (static) string according
*   to the format specified by the macro "elapsedTimeFormat".
*   Returns a pointer to the string.
*---------------------------------------------------------------------
*/

char *TIME_secs2elapsedTimeStr (TIME_t_seconds elapsed_time)
{
 int
   days,
   hours,
   mins;
 TIME_t_seconds
   secs;
 static t_elapsedTimeStr
   timeStr;

 days  = (int)  (elapsed_time / 86400);
 hours = (int) ((elapsed_time -= days * 86400) / 3600);
 mins  = (int) ((elapsed_time -= hours * 3600) / 60);
 secs  = (TIME_t_seconds)         elapsed_time - mins * 60;
 snprintf (timeStr, elapsedTimeStrSize, elapsedTimeFormat, days, hours, mins, secs);
 return  (timeStr);
}

/*
*---------------------------------------------------------------------
*   Delay. Total time is given by the parameter "seconds".
*---------------------------------------------------------------------
*/

void TIME_time_delay (TIME_t_seconds seconds)
{
 time_t
   current_time,
   starting_time;
 clock_t
   time_cycles,
   clock_cycles,
   clock_delay,
   current_clock,
   starting_clock;

 time_cycles = (clock_t) seconds;
 starting_time = current_time = time(NULL);
 while ((current_time - starting_time) < time_cycles)
   current_time = time(NULL);
 clock_cycles = (clock_t) ((seconds - time_cycles) * CLOCKS_PER_SECOND);
 starting_clock = current_clock = clock();
 clock_delay = current_clock - starting_clock;
 if (clock_delay < (clock_t) 0)
   clock_delay += (clock_t) CLOCK_CYCLE;
 while (clock_delay < clock_cycles) {
   current_clock = clock();
   clock_delay = current_clock - starting_clock;
   if (clock_delay < (clock_t) 0)
     clock_delay += (clock_t) CLOCK_CYCLE;
 }
}
