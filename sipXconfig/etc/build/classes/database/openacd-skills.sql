create table openacd_skill (
	openacd_skill_id integer not null,
	name character varying(255) not null unique,
	atom character varying(255) not null unique,
	group_name character varying(255) not null,
	description character varying(255),
	default_skill boolean not null default false,
	primary key (openacd_skill_id)
);
create sequence openacd_skill_seq;

insert into openacd_skill (openacd_skill_id, name, atom, group_name, description, default_skill)
	values (nextval('openacd_skill_seq'), 'English', 'english', 'Language', 'English', false);
insert into openacd_skill (openacd_skill_id, name, atom, group_name, description, default_skill)
	values (nextval('openacd_skill_seq'), 'German', 'german', 'Language', 'German', false);
insert into openacd_skill (openacd_skill_id, name, atom, group_name, description, default_skill)
	values (nextval('openacd_skill_seq'), 'Brand', '_brand', 'Magic',
			'Magic skill to expand to a client label (brand)', true);
insert into openacd_skill (openacd_skill_id, name, atom, group_name, description, default_skill)
	values (nextval('openacd_skill_seq'), 'Agent Name', '_agent', 'Magic',
			'Magic skill that is replaced by the agent name', true);
insert into openacd_skill (openacd_skill_id, name, atom, group_name, description, default_skill)
	values (nextval('openacd_skill_seq'), 'Agent Profile', '_profile', 'Magic',
			'Magic skill that is replaced by the agent profile name', true);
insert into openacd_skill (openacd_skill_id, name, atom, group_name, description, default_skill)
	values (nextval('openacd_skill_seq'), 'Node', '_node', 'Magic',
			'Magic skill that is replaced by the node identifier', true);
insert into openacd_skill (openacd_skill_id, name, atom, group_name, description, default_skill)
	values (nextval('openacd_skill_seq'), 'Queue', '_queue', 'Magic',
			'Magic skill replaced by a queue name', true);
insert into openacd_skill (openacd_skill_id, name, atom, group_name, description, default_skill)
	values (nextval('openacd_skill_seq'), 'All', '_all', 'Magic',
			'Magic skill to denote an agent that can answer any call regardless of the other skills', true);
