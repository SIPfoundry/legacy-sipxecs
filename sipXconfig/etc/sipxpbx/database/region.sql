create table region (
  region_id int4 not null,
  name varchar(255) unique,
  primary key (region_id)
);

create sequence region_seq;

alter table location add column region_id int4;
alter table location add constraint fk_users_branch
  foreign key (region_id)
  references region(region_id) match full
  on delete set null;

