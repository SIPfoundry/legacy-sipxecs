/* suppress warnings */
SET client_min_messages TO 'error';

create table version_history(
  version int4 not null primary key,
  applied date not null
);

insert into version_history (version, applied) values (1, now());

create table patch(
  name varchar(32) not null primary key
);
