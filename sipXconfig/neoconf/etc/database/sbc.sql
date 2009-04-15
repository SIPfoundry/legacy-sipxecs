create table sbc (
    sbc_id int4 not null,
    enabled bool,
    address varchar(255),
    primary key (sbc_id)
);

create table sbc_route_domain (
    sbc_id int4 not null,
    domain varchar(255) not null,
    index int4 not null,
    primary key (sbc_id, index)
);

create table sbc_route_subnet (
    sbc_id int4 not null,
    subnet varchar(255) not null,
    index int4 not null,
    primary key (sbc_id, index)
);

alter table sbc_route_domain 
    add constraint fk_sbc_route_domain_sbc 
    foreign key (sbc_id) 
    references sbc;

alter table sbc_route_subnet 
    add constraint fk_sbc_route_subnet_sbc 
    foreign key (sbc_id) 
    references sbc;

create sequence sbc_seq;
