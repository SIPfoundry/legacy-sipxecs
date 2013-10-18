create table openacd_queue_agent_group (
  openacd_agent_group_id integer not null,
  openacd_queue_id integer not null,
  constraint openacd_queue_agent_group_pkey primary key (openacd_agent_group_id, openacd_queue_id),
  constraint queue_agent_group_fk1 foreign key (openacd_agent_group_id)
      references openacd_agent_group (openacd_agent_group_id) match simple
      on update no action on delete no action,
  constraint queue_agent_group_fk2 foreign key (openacd_queue_id)
      references openacd_queue (openacd_queue_id) match simple
      on update no action on delete cascade
);

create table openacd_queue_agent (
  openacd_agent_id integer not null,
  openacd_queue_id integer not null,
  constraint openacd_queue_agent_pkey primary key (openacd_agent_id, openacd_queue_id),
  constraint queue_agent_fk1 foreign key (openacd_agent_id)
      references openacd_agent (openacd_agent_id) match simple
      on update no action on delete cascade,
  constraint queue_agent_fk2 foreign key (openacd_queue_id)
      references openacd_queue (openacd_queue_id) match simple
      on update no action on delete cascade
);

create table openacd_client_agent_group (
  openacd_agent_group_id integer not null,
  openacd_client_id integer not null,
  constraint openacd_client_agent_group_pkey primary key (openacd_agent_group_id, openacd_client_id),
  constraint client_agent_group_fk1 foreign key (openacd_agent_group_id)
      references openacd_agent_group (openacd_agent_group_id) match simple
      on update no action on delete cascade,
  constraint client_agent_group_fk2 foreign key (openacd_client_id)
      references openacd_client (openacd_client_id) match simple
      on update no action on delete cascade
);

create table openacd_client_agent (
  openacd_agent_id integer not null,
  openacd_client_id integer not null,
  constraint openacd_client_agent_pkey primary key (openacd_agent_id, openacd_client_id),
  constraint client_agent_fk1 foreign key (openacd_agent_id)
      references openacd_agent (openacd_agent_id) match simple
      on update no action on delete cascade,
  constraint client_agent_fk2 foreign key (openacd_client_id)
      references openacd_client (openacd_client_id) match simple
      on update no action on delete cascade
);

create table openacd_agent_group_queue_group (
  openacd_queue_group_id integer not null,
  openacd_agent_group_id integer not null,
  constraint openacd_agent_group_queue_group_pkey primary key (openacd_queue_group_id, openacd_agent_group_id),
  constraint agent_group_queue_group_fk1 foreign key (openacd_queue_group_id)
      references openacd_queue_group (openacd_queue_group_id) match simple
      on update no action on delete cascade,
  constraint agent_group_queue_group_fk2 foreign key (openacd_agent_group_id)
      references openacd_agent_group (openacd_agent_group_id) match simple
      on update no action on delete cascade
);

create table openacd_agent_group_queue (
  openacd_queue_id integer not null,
  openacd_agent_group_id integer not null,
  constraint openacd_agent_group_queue_pkey primary key (openacd_queue_id, openacd_agent_group_id),
  constraint agent_group_queue_fk1 foreign key (openacd_queue_id)
      references openacd_queue (openacd_queue_id) match simple
      on update no action on delete cascade,
  constraint agent_group_queue_fk2 foreign key (openacd_agent_group_id)
      references openacd_agent_group (openacd_agent_group_id) match simple
      on update no action on delete cascade
);