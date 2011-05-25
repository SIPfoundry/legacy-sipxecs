ALTER TABLE ftp_configuration DROP CONSTRAINT ftp_configuration_backup_plan_id_fkey;
ALTER TABLE ftp_configuration DROP COLUMN backup_plan_id;

ALTER TABLE backup_plan ADD COLUMN ftp_configuration_id int4;
ALTER TABLE backup_plan
    ADD CONSTRAINT fk_backup_plan_ftp_configuration
    FOREIGN KEY (ftp_configuration_id) REFERENCES ftp_configuration (id);
