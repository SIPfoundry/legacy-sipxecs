alter table location add column registered boolean;
update location set registered = 'true';
alter table location alter column registered set default false;