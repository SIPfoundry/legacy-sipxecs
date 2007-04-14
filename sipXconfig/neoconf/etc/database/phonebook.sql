create table phonebook (
   phonebook_id int4 not null,      
   name varchar(255) not null unique,
   description varchar(255),
   members_csv_filename varchar(255),
   primary key (phonebook_id)
);

create table phonebook_member (
   phonebook_id int4 not null,      
   group_id int4 not null,      
   primary key (phonebook_id, group_id)
);

create table phonebook_consumer (
   phonebook_id int4 not null,      
   group_id int4 not null,      
   primary key (phonebook_id, group_id)
);

create sequence phonebook_seq;

alter table phonebook_member add constraint phonebook_member_fk1 foreign key (phonebook_id) references phonebook;
alter table phonebook_member add constraint phonebook_member_fk2 foreign key (group_id) references group_storage;
alter table phonebook_consumer add constraint phonebook_consumer_fk1 foreign key (phonebook_id) references phonebook;
alter table phonebook_consumer add constraint phonebook_consumer_fk2 foreign key (group_id) references group_storage;
