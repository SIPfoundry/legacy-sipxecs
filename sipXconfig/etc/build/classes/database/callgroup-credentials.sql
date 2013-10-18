alter table call_group add column sip_password varchar(255);

-- trigger initializing newly added column
insert into initialization_task (name) values ('callgroup-password-init');
