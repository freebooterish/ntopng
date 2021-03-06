/*
 *
 * (C) 2013-17 - ntop.org
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "ntop_includes.h"

// #define ALERT_DEBUG 1

/* *************************************** */

AlertCounter::AlertCounter(u_int32_t _max_num_hits_sec,
			   u_int8_t _over_threshold_duration_sec) {
  max_num_hits_sec = _max_num_hits_sec;
  over_threshold_duration_sec = _over_threshold_duration_sec;
  if(over_threshold_duration_sec < 1) over_threshold_duration_sec = 1;
  init();
}

/* *************************************** */

void AlertCounter::init() {
  time_last_hit = time_last_alert_reported = 0;
  num_hits_rcvd_last_second = 0;
  last_trepassed_threshold = 0, num_trepassed_threshold = 0;
  num_hits_since_first_alert = 0;
}

/* *************************************** */

bool AlertCounter::incHits(time_t when) {
  if(time_last_hit < (when-1))
    init(); /* Only consecutive alerts matter */
  else if(time_last_hit == (when-1))
    num_hits_rcvd_last_second = 0;

  num_hits_rcvd_last_second++, num_hits_since_first_alert++, time_last_hit = when;

  if((num_hits_rcvd_last_second > max_num_hits_sec) 
     && (last_trepassed_threshold != when)) {
    num_trepassed_threshold++, last_trepassed_threshold = when;

#ifdef ALERT_DEBUG
    ntop->getTrace()->traceEvent(TRACE_NORMAL,
				 "Trepass [num: %u][last: %u][now: %u][duration: %u]",
				 num_trepassed_threshold, last_trepassed_threshold, when,
				 over_threshold_duration_sec);
#endif
          
    if(num_trepassed_threshold > over_threshold_duration_sec) {
#ifdef ALERT_DEBUG
      ntop->getTrace()->traceEvent(TRACE_NORMAL,
				   "Alarm triggered [num: %u][last: %u][now: %u][duration: %u]",
				   num_trepassed_threshold, last_trepassed_threshold, when,
				   over_threshold_duration_sec);
#endif

      if(when > (time_last_alert_reported+CONST_ALERT_GRACE_PERIOD)) {
#ifdef ALERT_DEBUG
	ntop->getTrace()->traceEvent(TRACE_NORMAL, 
				     "Alert emitted [num: %u][now: %u][duration: %u][tot_hits: %u]",
				     num_trepassed_threshold, when, 
				     over_threshold_duration_sec, num_hits_since_first_alert);
#endif
	time_last_alert_reported = when;
	return(true);
      }
    }
  }  

  return(false);
}
