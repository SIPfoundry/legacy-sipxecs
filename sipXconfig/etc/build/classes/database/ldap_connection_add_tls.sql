alter table ldap_connection add column use_tls boolean;
update ldap_connection set use_tls = false;
alter table ldap_connection alter column use_tls set not null;
alter table ldap_connection alter column use_tls set default false;
