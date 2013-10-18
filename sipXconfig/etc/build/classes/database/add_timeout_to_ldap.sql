alter table ldap_connection add column timeout integer;
update ldap_connection set timeout = 10000;