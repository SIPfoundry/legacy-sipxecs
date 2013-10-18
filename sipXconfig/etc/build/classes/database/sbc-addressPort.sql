alter table sbc
  add column address_actual varchar(255),
  add column port integer;
alter table sbc alter column port set default 5060;

alter table gateway
add column use_sipxbridge bool,
add column outbound_address varchar(255),
add column outbound_port integer;
alter table gateway alter column use_sipxbridge set default true;
alter table gateway alter column outbound_port set default 5060;
