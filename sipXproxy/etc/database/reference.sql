alter table call_state_events
  add column reference text;
alter table cdrs
  add column reference text;
