create table openacd_queue_group (
	openacd_queue_group_id integer not null,
	name character varying(255) not null unique,
	sort integer not null,
	description character varying(255),
	primary key (openacd_queue_group_id)
);
create sequence openacd_queue_group_seq;

create table openacd_skill_queue_group (
  openacd_queue_group_id integer not null,
  openacd_skill_id integer not null,
  constraint openacd_skill_queue_group_pkey primary key (openacd_queue_group_id, openacd_skill_id),
  constraint skill_queue_group_fk1 foreign key (openacd_queue_group_id)
      references openacd_queue_group (openacd_queue_group_id) match simple
      on update no action on delete no action,
  constraint skill_queue_group_fk2 foreign key (openacd_skill_id)
      references openacd_skill (openacd_skill_id) match simple
      on update no action on delete no action
);

create table openacd_queue (
	openacd_queue_id integer not null,
	name character varying(255) not null unique,
	description character varying(255),
	openacd_queue_group_id integer not null,
	weight integer not null,
	primary key (openacd_queue_id),
	constraint fk_openacd_queue_group foreign key (openacd_queue_group_id)
      references openacd_queue_group (openacd_queue_group_id) match simple
      on update no action on delete no action
);
create sequence openacd_queue_seq;

create table openacd_skill_queue (
  openacd_queue_id integer not null,
  openacd_skill_id integer not null,
  constraint openacd_skill_queue_pkey primary key (openacd_queue_id, openacd_skill_id),
  constraint skill_queue_fk1 foreign key (openacd_queue_id)
      references openacd_queue (openacd_queue_id) match simple
      on update no action on delete no action,
  constraint skill_queue_fk2 foreign key (openacd_skill_id)
      references openacd_skill (openacd_skill_id) match simple
      on update no action on delete no action
);

insert into openacd_queue_group (openacd_queue_group_id, name, sort, description)
	values (nextval('openacd_queue_group_seq'), 'Default', 0, 'Default queue group');

insert into openacd_queue (openacd_queue_id, name, description, openacd_queue_group_id, weight)
	values (nextval('openacd_queue_seq'), 'default_queue', 'Default queue',
		(select openacd_queue_group_id from openacd_queue_group where name = 'Default'), 1);
