/*
 * Instead of providing two different views of CDRs, one of which has call direction and
 * one that doesn't, just define a single view, "view_cdrs" that includes everything.  If
 * call direction is not being computed, then that column will always be null, which is OK.
 */

/* Drop both views */
drop view view_cdrs;
drop view view_cdrs_with_call_direction;

/* Recreate view_cdrs to be just like view_cdrs_with_call_direction was */
create view view_cdrs as
  select id, caller_aor, callee_aor,
         start_time, connect_time, end_time,
         end_time - connect_time as duration,
         termination, failure_status, failure_reason,
         call_direction
  from cdrs;
