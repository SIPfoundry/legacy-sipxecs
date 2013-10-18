alter table freeswitch_extension drop column location_id;
alter table freeswitch_extension add column enabled boolean not null default true;

-- login
insert into freeswitch_extension (freeswitch_ext_id, name, description, freeswitch_ext_type, did, alias, enabled)
  values (nextval('freeswitch_ext_seq'), 'login', 'Default login dial string', 'C', NULL, NULL, false);
insert into freeswitch_condition (freeswitch_condition_id, field, expression, freeswitch_ext_id)
  values (nextval('freeswitch_condition_seq'), 'destination_number', '^*87$', (select currval('freeswitch_ext_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'answer', NULL, (select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'erlang_sendmsg',
	'agent_dialplan_listener openacd@' || (select fqdn from location where primary_location = true) || ' agent_login ${sip_from_user} pstn ${sip_from_uri}',
	(select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'sleep', '2000', (select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'hangup', 'NORMAL_CLEARING', (select currval('freeswitch_condition_seq')));

-- available
insert into freeswitch_extension (freeswitch_ext_id, name, description, freeswitch_ext_type, did, alias, enabled)
  values (nextval('freeswitch_ext_seq'), 'available', 'Default available dial string', 'C', NULL, NULL, false);
insert into freeswitch_condition (freeswitch_condition_id, field, expression, freeswitch_ext_id)
  values (nextval('freeswitch_condition_seq'), 'destination_number', '^*90$', (select currval('freeswitch_ext_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'answer', NULL, (select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'erlang_sendmsg',
	'agent_dialplan_listener openacd@' || (select fqdn from location where primary_location = true) || ' agent_available ${sip_from_user}',
	(select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'sleep', '2000', (select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'hangup', 'NORMAL_CLEARING', (select currval('freeswitch_condition_seq')));

-- release
insert into freeswitch_extension (freeswitch_ext_id, name, description, freeswitch_ext_type, did, alias, enabled)
  values (nextval('freeswitch_ext_seq'), 'release', 'Default release dial string', 'C', NULL, NULL, false);
insert into freeswitch_condition (freeswitch_condition_id, field, expression, freeswitch_ext_id)
  values (nextval('freeswitch_condition_seq'), 'destination_number', '^*91$', (select currval('freeswitch_ext_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'answer', NULL, (select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'erlang_sendmsg',
	'agent_dialplan_listener openacd@' || (select fqdn from location where primary_location = true) || ' agent_release ${sip_from_user}',
	(select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'sleep', '2000', (select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'hangup', 'NORMAL_CLEARING', (select currval('freeswitch_condition_seq')));

-- logoff
insert into freeswitch_extension (freeswitch_ext_id, name, description, freeswitch_ext_type, did, alias, enabled)
  values (nextval('freeswitch_ext_seq'), 'logoff', 'Default logoff dial string', 'C', NULL, NULL, false);
insert into freeswitch_condition (freeswitch_condition_id, field, expression, freeswitch_ext_id)
  values (nextval('freeswitch_condition_seq'), 'destination_number', '^*89$', (select currval('freeswitch_ext_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'answer', NULL, (select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'erlang_sendmsg',
	'agent_dialplan_listener openacd@' || (select fqdn from location where primary_location = true) || ' agent_logoff ${sip_from_user}',
	(select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'sleep', '2000', (select currval('freeswitch_condition_seq')));
insert into freeswitch_action (freeswitch_action_id, application, data, freeswitch_condition_id)
  values (nextval('freeswitch_action_seq'), 'hangup', 'NORMAL_CLEARING', (select currval('freeswitch_condition_seq')));
