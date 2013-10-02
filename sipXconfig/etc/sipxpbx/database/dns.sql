
create table dns_plan (
  dns_plan_id int4 not null,
  name varchar(255) not null unique,  
  primary key (dns_plan_id)
);

create table dns_group (  
  dns_group_id int4 not null,
  dns_plan_id int4 references dns_plan on delete cascade,
  position int not null,
  primary key (dns_group_id)
);

-- union or region or location or basic
create table dns_target (
  dns_group_id int4 not null references dns_group on delete cascade,
  percentage int4 not null,
  region_id int4 references region,
  location_id int4 references location,
  basic_id char(1)
);

create table dns_view (
  dns_view_id int4 not null,
  name varchar(255) not null unique,
  enabled bool not null default true,
  dns_plan_id int4 references dns_plan,
  region_id int4 references region,
  position int not null
);

create sequence dns_view_seq;
create sequence dns_plan_seq;
create sequence dns_group_seq;
