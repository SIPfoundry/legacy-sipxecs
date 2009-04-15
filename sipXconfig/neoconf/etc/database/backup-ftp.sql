CREATE TABLE ftp_configuration
(
  id int4 NOT NULL,
  host varchar(255),
  user_id varchar(255),
  "password" varchar(255),
  backup_plan_id int4 NOT NULL,
  CONSTRAINT ftp_configuration_pkey PRIMARY KEY (id),
  CONSTRAINT ftp_configuration_backup_plan_id_fkey FOREIGN KEY (backup_plan_id)
      REFERENCES backup_plan (backup_plan_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
);

create sequence ftp_configuration_seq;

ALTER TABLE backup_plan ADD COLUMN backup_type char;
UPDATE backup_plan SET backup_type = 'L';
ALTER TABLE backup_plan ALTER COLUMN backup_type SET NOT NULL;
