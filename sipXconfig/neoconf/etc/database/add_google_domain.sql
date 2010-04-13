create table google_domain (
  google_domain_id integer not null,
  domain_name character varying(255),
  constraint google_domain_pkey primary key (google_domain_id)
);

create sequence google_domain_seq;

insert into google_domain values (nextval('google_domain_seq'), 'gmail.com');
