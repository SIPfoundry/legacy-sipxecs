alter table attendant_dialing_rule add column live_attendant_enabled boolean default true;
alter table attendant_dialing_rule add column live_attendant_code character varying(255);
alter table attendant_dialing_rule add live_attendant_expire timestamp;
