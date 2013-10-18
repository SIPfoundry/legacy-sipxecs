create table shard (
    shard_id int4 not null,
    name varchar(255) not null unique,
    primary key (shard_id)
);

create sequence shard_seq;
