create table openacd_skill_group (
	openacd_skill_group_id integer not null,
	name character varying(255) not null unique,
	description character varying(255),
	primary key (openacd_skill_group_id)
);
create sequence openacd_skill_group_seq;

-- modify 'openacd_skill' table to add 'openacd_skill_group_id' column
alter table openacd_skill add column openacd_skill_group_id integer;

-- modify 'openacd_skill' table to add foreign key constraint to 'openacd_skill_group' table
alter table openacd_skill add constraint fk_openacd_skill_group foreign key (openacd_skill_group_id)
      references openacd_skill_group (openacd_skill_group_id) match full;

insert into initialization_task (name) values ('skill_group_name_migrate_skill_group');
