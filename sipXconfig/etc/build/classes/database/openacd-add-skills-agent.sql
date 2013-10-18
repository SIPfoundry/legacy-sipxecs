create table openacd_skill_agent_group (
  openacd_agent_group_id integer not null,
  openacd_skill_id integer not null,
  constraint openacd_skill_agent_group_pkey primary key (openacd_agent_group_id, openacd_skill_id),
  constraint skill_agent_group_fk1 foreign key (openacd_agent_group_id)
      references openacd_agent_group (openacd_agent_group_id) match simple
      on update no action on delete no action,
  constraint skill_agent_group_fk2 foreign key (openacd_skill_id)
      references openacd_skill (openacd_skill_id) match simple
      on update no action on delete no action
);

create table openacd_skill_agent (
  openacd_agent_id integer not null,
  openacd_skill_id integer not null,
  constraint openacd_skill_agent_pkey primary key (openacd_agent_id, openacd_skill_id),
  constraint skill_agent_fk1 foreign key (openacd_agent_id)
      references openacd_agent (openacd_agent_id) match simple
      on update no action on delete no action,
  constraint skill_agent_fk2 foreign key (openacd_skill_id)
      references openacd_skill (openacd_skill_id) match simple
      on update no action on delete no action
);