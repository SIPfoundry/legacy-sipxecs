alter table users add column notified boolean;
update users set notified = false;
alter table users alter column notified set not null;
alter table users alter column notified set default false;