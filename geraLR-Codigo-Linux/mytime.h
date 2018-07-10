/*
*-----------------------------------------------------------------------
*
*   File         : time.h
*   Created      : 1994-05-19
*   Last Modified: 2012-05-28
*
*   Time-related functions and stopwatches
*
*-----------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _MYTIME_DOT_H_
#define _MYTIME_DOT_H_

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <time.h>

#define TIME_t_stopwatch  int
#define TIME_t_seconds    double
#define TIME_NULL_TIME    ((TIME_t_seconds) -1)

/*
*---------------------------------------------------------------------
*   Procedures defined in "timing.c"
*---------------------------------------------------------------------
*/

extern void              TIME_init_stopwatches    (void);
extern TIME_t_stopwatch  TIME_setup_stopwatch     (void);
extern TIME_t_seconds    TIME_read_stopwatch      (TIME_t_stopwatch index);
extern TIME_t_seconds    TIME_stop_stopwatch      (TIME_t_stopwatch index);
extern TIME_t_seconds    TIME_start_stopwatch     (TIME_t_stopwatch index);
extern TIME_t_seconds    TIME_clear_stopwatch     (TIME_t_stopwatch index);
extern TIME_t_seconds    TIME_reset_stopwatch     (TIME_t_stopwatch index);
extern TIME_t_seconds    TIME_dismiss_stopwatch   (TIME_t_stopwatch index);
extern char             *TIME_startTime_stopwatch (TIME_t_stopwatch index);
extern char             *TIME_getCurrentTimeStr   (void);
extern char             *TIME_time2realTimeStr    (time_t real_time);
extern char             *TIME_secs2elapsedTimeStr (TIME_t_seconds elapsed_time);
extern void              TIME_time_delay          (TIME_t_seconds seconds);

#endif /* ifndef _MYTIME_DOT_H_ */
