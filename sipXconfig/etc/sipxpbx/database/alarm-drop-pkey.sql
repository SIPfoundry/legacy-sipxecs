-- when freezing - remove alarm_seq_id column and add pkey to alarm_code_id 
alter table alarm_code drop constraint alarm_code_pkey;
alter table alarm_code drop column alarm_seq_id; 
alter table alarm_code add primary key (alarm_code_id);
