create table supervisor(
   user_id int4 not null,
   group_id int4 not null,
   primary key (user_id, group_id)
);  
alter table supervisor add constraint supervisor_fk1 foreign key (user_id) references users;
alter table supervisor add constraint supervisor_fk2 foreign key (group_id) references group_storage;
