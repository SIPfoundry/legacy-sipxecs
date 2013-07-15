create table call_queue_agent(
    call_queue_agent_id int4 not null primary key,
    name varchar(255) not null unique,
    extension varchar(255),
    description varchar(255),
    value_storage_id int4
);

create table call_queue_tier(
    call_queue_agent_id int4,
    freeswitch_ext_id int4,
    position int4,
    level int4
);

create sequence call_queue_agent_seq;

alter table call_queue_agent add constraint call_queue_agent_value_storage foreign key (value_storage_id)    references value_storage;
alter table call_queue_tier  add constraint freeswitch_ext_id              foreign key (freeswitch_ext_id)   references freeswitch_extension;
alter table call_queue_tier  add constraint call_queue_tier_agent          foreign key (call_queue_agent_id) references call_queue_agent;
alter table call_queue_tier  add constraint call_queue_tier_agent_queue unique (freeswitch_ext_id,call_queue_agent_id);
