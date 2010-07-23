create sequence ldap_settings_seq;

create table ldap_settings (
  ldap_settings_id integer not null,
  authentication_options varchar(8) not null default 'noLDAP',
  enable_openfire_configuration bool not null,
  constraint ldap_settings_pkey primary key (ldap_settings_id)
);
