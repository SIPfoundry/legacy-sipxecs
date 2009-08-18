alter table call_state_events add column caller_internal boolean;
alter table call_state_events add column callee_route text ;

alter table cdrs add column caller_internal boolean;
alter table cdrs add column callee_route text ;
