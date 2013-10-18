CREATE TABLE nat_traversal
(
  id int4 NOT NULL,
  enabled bool,
  behindnat bool,
  value_storage_id int4,
  CONSTRAINT nat_traversal_pkey PRIMARY KEY (id),
  CONSTRAINT nat_traversal_value_storage_id_fkey FOREIGN KEY (value_storage_id)
      REFERENCES value_storage (value_storage_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
);

insert into initialization_task (name) values ('default_NAT_traversal');
