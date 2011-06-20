create table localization (
   localization_id int4 not null,
   "region" character varying(255),
   "language" character varying(255),
   primary key (localization_id)
);
create sequence localization_seq;
alter table dial_plan add type character varying(255);

