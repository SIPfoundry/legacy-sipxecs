create table user_external_im_account
(
  external_im_account_id integer not null,
  user_id integer not null,
  type character varying(255),
  username character varying(255),
  password character varying(255),
  display_name character varying(255),
  enabled boolean not null default true,
  constraint user_external_im_account_pkey primary key (external_im_account_id),
  constraint fk_user_external_im_account foreign key (user_id) references users
);

-- create sequence for user_external_im_account table
create sequence external_im_account_seq;
