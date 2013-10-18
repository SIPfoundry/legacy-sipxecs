create table speeddial (
    speeddial_id int4 not null,
    user_id int4 not null,
    primary key (speeddial_id)
);

create table speeddial_button (
    speeddial_id int4 not null,
    label varchar(255),
    number varchar(255) not null,
    blf boolean not null default false,
    position int4 not null,
    primary key (speeddial_id, position)
);

alter table speeddial 
    add constraint fk_speeddial_user 
    foreign key (user_id) 
    references users;

alter table speeddial_button
  add constraint fk_speeddial_button_speeddial
  foreign key (speeddial_id)
  references speeddial;

create sequence speeddial_seq;