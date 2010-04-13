-- alter paging group table
alter table paging_group drop column prefix;

-- create paging_group table
create table paging_server
(
  paging_server_id integer not null,
  prefix character varying(255) not null,
  constraint paging_server_pkey primary key (paging_server_id)
);

