create table private_user_key (
  private_user_key_id int4 not null,
  key varchar(255) unique,
  user_id int4,
  primary key (private_user_key_id)
);

create sequence private_user_key_seq;

alter table private_user_key add constraint fk_private_user_key_user
  foreign key (user_id)
  references users(user_id) match full
  on delete cascade;
