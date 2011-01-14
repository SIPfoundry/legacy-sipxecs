create table openacd_client (
	openacd_client_id integer not null,
	name character varying(255) not null unique,
	identity character varying(255) not null unique,
	description character varying(255),
	primary key (openacd_client_id)
);
create sequence openacd_client_seq;