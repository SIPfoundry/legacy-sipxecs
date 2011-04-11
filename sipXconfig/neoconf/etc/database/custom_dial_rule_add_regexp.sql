alter table custom_dialing_rule add column regex boolean;
update custom_dialing_rule set regex = false;
alter table custom_dialing_rule alter column regex set not null;
alter table custom_dialing_rule alter column regex set default false;
