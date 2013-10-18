create table openacd_release_codes (
	openacd_code_id integer not null,
	label character varying(255) not null unique,
	bias character varying(255) not null,
	description character varying(255),
	primary key (openacd_code_id)
);
create sequence openacd_release_codes_seq;
