create table openacd_recipe_step (
	openacd_recipe_step_id integer not null,
	name character varying(255),
	description character varying(255),
	openacd_recipe_action_id integer unique,
	frequency character varying(255) not null,
	openacd_queue_id integer not null,
	primary key (openacd_recipe_step_id),
	constraint fk_openacd_queue foreign key (openacd_queue_id)
	  references openacd_queue (openacd_queue_id) match simple
);
create sequence openacd_recipe_step_seq;

create table openacd_recipe_action (
	openacd_recipe_action_id integer not null,
	action character varying(255) not null,
	action_value character varying(255),
	primary key (openacd_recipe_action_id)
);
create sequence openacd_recipe_action_seq;

create table openacd_skill_recipe_action (
	openacd_recipe_action_id integer not null,
	openacd_skill_id integer not null,
	constraint openacd_skill_recipe_action_pkey primary key (openacd_recipe_action_id, openacd_skill_id),
	constraint skill_recipe_action_fk1 foreign key (openacd_recipe_action_id)
	  references openacd_recipe_action (openacd_recipe_action_id) match simple
	  on update no action on delete no action,
	constraint skill_recipe_action_fk2 foreign key (openacd_skill_id)
	  references openacd_skill (openacd_skill_id) match simple
	  on update no action on delete no action
);

create table openacd_recipe_condition (
	openacd_recipe_step_id integer not null,
	condition character varying(255) not null,
	relation character varying(255) not null,
	value_condition character varying(255) not null,
	index integer,
	constraint recipe_condition_pkey primary key (openacd_recipe_step_id, index)
);
