create table domain (
   domain_id int4 not null,   
   name varchar(255) not null,
   primary key (domain_id)
);

create sequence domain_seq;

create table domain_alias (
   domain_id int4 not null,
   alias varchar(255) not null,
   primary key (domain_id, alias)   
);

-- make sure that the domain is created on system startup
insert into initialization_task (name) values ('initialize-domain');
