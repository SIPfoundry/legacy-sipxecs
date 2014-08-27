DROP SEQUENCE IF EXISTS config_change_seq;
DROP SEQUENCE IF EXISTS config_change_value_seq;
DROP TABLE IF EXISTS config_change_value;
DROP TABLE IF EXISTS config_change;

CREATE SEQUENCE config_change_seq;

CREATE TABLE config_change
(
  config_change_id integer NOT NULL,
  date_time timestamp without time zone NOT NULL,
  user_name character varying(255) NOT NULL,
  ip_address character varying(20) NOT NULL,
  config_change_type character varying(255) NOT NULL,
  config_change_action character varying(10) NOT NULL,
  details character varying(255),
  CONSTRAINT config_change_id_pkey PRIMARY KEY (config_change_id )
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
