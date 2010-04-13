create table speeddial_group (
    speeddial_id int4 not null,
    group_id int4 not null,
    primary key (speeddial_id)
);

create table speeddial_group_button (
    speeddial_id int4 not null,
    label varchar(255),
    number varchar(255) not null,
    blf boolean not null default false,
    position int4 not null,
    primary key (speeddial_id, position)
);

alter table speeddial_group
  add constraint fk_speeddial_group
  foreign key (group_id)
  references group_storage;

alter table speeddial_group_button
  add constraint fk_speeddial_button_speeddial
  foreign key (speeddial_id)
  references speeddial_group;

create sequence speeddial_group_seq;