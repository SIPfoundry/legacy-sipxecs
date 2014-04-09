CREATE SEQUENCE user_ip_address_seq;

CREATE TABLE user_ip_address
(
  user_ip_address_id integer NOT NULL,
  user_name character varying(255) NOT NULL,
  ip_address character varying(255) NOT NULL,
  CONSTRAINT user_ip_address_pkey PRIMARY KEY (user_ip_address_id),
  CONSTRAINT user_name_ip_address UNIQUE (user_name, ip_address)
);
