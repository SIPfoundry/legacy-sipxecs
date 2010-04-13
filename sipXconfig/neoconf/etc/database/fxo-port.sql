create table fxo_port (
  fxo_port_id int4 not null,
  gateway_id int4 not null,
  position int4,
  value_storage_id int4,
  primary key (fxo_port_id)
);

alter table fxo_port
  add constraint fk_fxo_port_gateway
  foreign key (gateway_id)
  references gateway;

alter table fxo_port
  add constraint fk_fxo_port_value_storage
  foreign key (value_storage_id)
  references value_storage;

create sequence fxo_port_seq;
