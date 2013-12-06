create table dns_custom (
  dns_custom_id int4 not null,
  name varchar(255) not null unique,
  records text,
  primary key (dns_custom_id)
);

create table dns_custom_view_link (
  dns_custom_id int4 not null,
  dns_view_id int4 not null,
  primary key (dns_custom_id, dns_view_id)	
);

alter table dns_view add 
  excluded varchar(32)
;

create sequence dns_custom_seq;
