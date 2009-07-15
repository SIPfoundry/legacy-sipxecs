alter table users add column is_shared boolean;
update users set is_shared = false;
alter table users alter column is_shared set not null;
alter table users alter column is_shared set default false;