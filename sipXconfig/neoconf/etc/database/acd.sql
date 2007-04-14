create table acd_agent (
    acd_agent_id int4 not null,
    value_storage_id int4,
    user_id int4 not null,
    primary key (acd_agent_id)
);

create table acd_line (
    acd_line_id int4 not null,
    name varchar(255) not null,
    description varchar(255),
    value_storage_id int4,
    acd_server_id int4 not null,
    primary key (acd_line_id)
);

create table acd_line_queue (
    acd_line_id int4 not null,
    acd_queue_id int4 not null,
    primary key (acd_line_id)
);

create table acd_queue (
    acd_queue_id int4 not null,
    name varchar(255) not null,
    description varchar(255),
    value_storage_id int4,
    acd_server_id int4 not null,
    primary key (acd_queue_id)
);

create table acd_queue_agent (
    acd_queue_id int4 not null,
    acd_agent_id int4 not null,
    agent_position int4,
    primary key (acd_queue_id, agent_position)
);

create table acd_agent_queue (
    acd_queue_id int4 not null,
    acd_agent_id int4 not null,
    queue_position int4 not null,
    primary key (acd_agent_id, queue_position)
);

create table acd_server (
    acd_server_id int4 not null,
    host varchar(255),
    port int4,
    description varchar(255),
    value_storage_id int4,
    primary key (acd_server_id)
);

alter table acd_queue_agent
    add constraint fk_acd_queue_agent_queue_id
    foreign key (acd_queue_id) 
    references acd_queue;
alter table acd_queue_agent 
    add constraint fk_acd_queue_agent_agent_id 
    foreign key (acd_agent_id) 
    references acd_agent;
    
alter table acd_agent_queue
    add constraint fk_acd_agent_queue_queue_id
    foreign key (acd_queue_id) 
    references acd_queue;
alter table acd_agent_queue 
    add constraint fk_acd_agent_queue_agent_id 
    foreign key (acd_agent_id) 
    references acd_agent;
    
alter table acd_agent 
    add constraint FKAB953508A53EB3E4 
    foreign key (user_id) 
    references users;
alter table acd_agent 
    add constraint FKAB9535086C899BCE 
    foreign key (value_storage_id) 
    references value_storage;
alter table acd_line 
    add constraint FK816CF191D04BF1FA 
    foreign key (acd_server_id) 
    references acd_server;
alter table acd_line 
    add constraint FK816CF1916C899BCE 
    foreign key (value_storage_id) 
    references value_storage;
alter table acd_line_queue 
    add constraint FK1D273CE31DACB05A 
    foreign key (acd_line_id) 
    references acd_line;
alter table acd_line_queue 
    add constraint FK1D273CE3BEE3CFA 
    foreign key (acd_queue_id) 
    references acd_queue;
alter table acd_queue 
    add constraint FKAC7D0B14D04BF1FA 
    foreign key (acd_server_id) 
    references acd_server;
alter table acd_queue 
    add constraint FKAC7D0B146C899BCE 
    foreign key (value_storage_id) 
    references value_storage;
alter table acd_server 
    add constraint FKE5B27DA06C899BCE 
    foreign key (value_storage_id) 
    references value_storage;

create sequence acd_seq;
