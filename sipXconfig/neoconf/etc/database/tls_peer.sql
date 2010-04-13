create sequence tls_peer_seq;

create table tls_peer
(
  tls_peer_id integer NOT NULL,
  "name" character varying(255) NOT NULL,
  internal_user_id integer,
  constraint tls_peer_pkey primary key (tls_peer_id),
  constraint fk_internal_user foreign key (internal_user_id)
      references users (user_id) match simple
      on update no action on delete no action
);
