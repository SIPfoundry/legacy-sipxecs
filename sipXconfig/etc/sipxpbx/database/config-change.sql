CREATE SEQUENCE config_change_seq;

CREATE TABLE config_change
(
  config_change_id integer NOT NULL,
  date_time timestamp without time zone NOT NULL,
  user_ip_address_id integer NOT NULL,
  config_change_type character varying(10) NOT NULL,
  config_change_action character varying(10) NOT NULL,
  details character varying,
  CONSTRAINT config_change_id_pkey PRIMARY KEY (config_change_id ),
  CONSTRAINT config_change_fk1 FOREIGN KEY (user_ip_address_id)
      REFERENCES user_ip_address (user_ip_address_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
);

CREATE SEQUENCE config_change_value_seq;

CREATE TABLE config_change_value
(
  config_change_value_id integer NOT NULL,
  config_change_id integer NOT NULL,
  property_name character varying(255),
  value_before character varying(255),
  value_after character varying(255),
  CONSTRAINT config_change_value_pkey PRIMARY KEY (config_change_value_id),
  CONSTRAINT config_change_value_fk1 FOREIGN KEY (config_change_id)
      REFERENCES config_change (config_change_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE CASCADE
);
