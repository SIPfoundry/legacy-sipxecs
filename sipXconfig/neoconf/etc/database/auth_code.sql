create sequence auth_code_seq;

create table auth_code 
(
  auth_code_id integer NOT NULL,
  "code" character varying(255) NOT NULL,
  "description" varchar(255),
  internal_user_id integer,
  constraint auth_code_pkey primary key (auth_code_id),
  constraint fk_internal_user foreign key (internal_user_id)
      references users (user_id) match simple
      on update no action on delete no action
);
