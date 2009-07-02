-- add per-user call forward timer
alter table users add column cfwd_time int4;
update users set cfwd_time = 20;
alter table users alter column cfwd_time set not null;
alter table users alter column cfwd_time set default 20;
