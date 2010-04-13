alter table sbc_device add column port int4;
alter table sbc_device alter column port set default 5060;
update sbc_device set port = 5060;
alter table sbc_device alter column port set not null;