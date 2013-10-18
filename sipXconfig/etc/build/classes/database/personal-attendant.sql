create table personal_attendant (
   personal_attendant_id int4 not null,
   user_id int4 not null,
   operator varchar(255),
   primary key (personal_attendant_id)
);

create table personal_attendant_menu_item (
   personal_attendant_id int4 not null,
   action varchar(255),
   parameter varchar(255),
   dialpad_key varchar(255) not null,
   primary key (personal_attendant_id, dialpad_key)
);


alter table personal_attendant add constraint  fk_personal_atendant_users
  foreign key (user_id) references users; 

alter table personal_attendant_menu_item add constraint fk_personal_attendant_menu_item_personal_attendant 
  foreign key (personal_attendant_id) references personal_attendant;

create sequence personal_attendant_seq;
