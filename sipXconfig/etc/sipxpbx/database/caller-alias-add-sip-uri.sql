-- additional gateway columns
alter table gateway add column enable_caller_id boolean not null default false;
alter table gateway add column caller_id varchar(255);
alter table gateway add column display_name varchar(255);
alter table gateway add column url_parameters varchar(255);