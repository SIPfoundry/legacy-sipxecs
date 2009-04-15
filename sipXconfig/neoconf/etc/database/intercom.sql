/*
 * Intercom configuration:
 *   prefix - the prefix for placing an intercom call
 *   timeout (in milliseconds) - how long the phone rings before the call is automatically answered
 *   code - a string sent to the phone that identifies the intercom configuration
 */
create table intercom (
   intercom_id int4 not null,
   enabled bool not null,
   prefix varchar(255) not null unique,
   timeout int4 not null,
   code varchar(255) not null unique,
   primary key (intercom_id)
);

/*
 * intercom_phone_group is a join table that links an intercom configuration
 * to one or more phone groups
 */
create table intercom_phone_group (
   intercom_id int4 not null,
   group_id int4 not null,
   primary key (intercom_id, group_id)
);


/*
 * Foreign key constraints
 */
 
alter table intercom_phone_group
  add constraint intercom_phone_group_fk1
  foreign key (intercom_id)
  references intercom;
  
alter table intercom_phone_group
  add constraint intercom_phone_group_fk2
  foreign key (group_id)
  references group_storage;


/*
 * Sequence used to create IDs sequentially
 */
create sequence intercom_seq;
