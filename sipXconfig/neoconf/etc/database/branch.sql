create table branch (
  branch_id int4 not null,
  name varchar(255) unique,
  description varchar(255),
  primary key (branch_id)
);

create sequence branch_seq;

-- branch for users
alter table users add column branch_id int4;
alter table users add constraint fk_users_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;

-- branch for phones
alter table phone add column branch_id int4;
alter table phone add constraint fk_phone_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;

-- branch for users and phone groups
alter table group_storage add column branch_id int4;
alter table group_storage add constraint fk_group_storage_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;

-- replace site_id with branch_id for gateways
alter table gateway drop constraint fk_gateway_site;
alter table gateway drop column site_id;
alter table gateway add column branch_id int4;
alter table gateway add constraint fk_gateway_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;

-- branch for location/server
alter table location add column branch_id int4;
alter table location add constraint fk_location_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;

-- branch for SBC
alter table sbc_device add column branch_id int4;
alter table sbc_device add constraint fk_sbc_device_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;
