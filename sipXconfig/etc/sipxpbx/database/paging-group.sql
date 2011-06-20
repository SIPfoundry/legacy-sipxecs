-- create paging_group table
create table paging_group
(
  paging_group_id integer not null,
  page_group_number integer not null,
  description character varying(255),
  enabled boolean not null default false,
  sound character varying(255) not null,
  prefix character varying(255) not null,
  constraint paging_group_pkey primary key (paging_group_id)
);

-- create user_paging_group table
create table user_paging_group
(
  paging_group_id integer not null,
  user_id integer not null,
  constraint user_paging_group_pkey primary key (paging_group_id, user_id),
  constraint paging_group_fk1 FOREIGN KEY (paging_group_id)
      references paging_group (paging_group_id) match simple
      on update no action on delete no action,
  constraint paging_group_fk2 FOREIGN KEY (user_id)
      references users (user_id) match simple
      on update no action on delete no action
);

-- create sequence for paging_group table
create sequence paging_group_seq;