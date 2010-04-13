/*
 * Add indexes to CDR and CSE tables
 */

create index call_state_events_event_time on call_state_events (event_time) ;
create index cdrs_start_time_idx on cdrs (start_time) ;
