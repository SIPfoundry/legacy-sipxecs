alter table internal_dialing_rule add column did character varying(255);
alter table attendant_dialing_rule add column did character varying(255);
alter table meetme_conference add column did character varying(255);
alter table call_group add column did character varying(255);
alter table acd_line add column did character varying(255);
