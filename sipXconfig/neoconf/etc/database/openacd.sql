create table openacd_agent_group (
	openacd_agent_group_id integer not null,
	name character varying(255) not null unique,
	description character varying(255),
	primary key (openacd_agent_group_id)
);
create sequence openacd_agent_group_seq;

create table openacd_agent (
	openacd_agent_id integer not null,
	openacd_agent_group_id integer not null,
	user_id integer NOT NULL,
	pin character varying(255) not null,
	security character varying(255) not null,
	primary key (openacd_agent_id),
	constraint fk_openacd_agent_group foreign key (openacd_agent_group_id)
      references openacd_agent_group (openacd_agent_group_id) match simple
      on update no action on delete no action,
	constraint fk_user_id foreign key (user_id)
      references users (user_id) match SIMPLE
      on update no action on delete no action
);
create sequence openacd_agent_seq;

insert into openacd_agent_group (openacd_agent_group_id, name, description)
	values (nextval('openacd_agent_group_seq'), 'Default', 'Default agent group');
