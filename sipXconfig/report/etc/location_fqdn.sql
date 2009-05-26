ALTER TABLE acd_agent_stat
   ADD COLUMN location_fqdn character varying(255);
ALTER TABLE acd_call_stat
   ADD COLUMN location_fqdn character varying(255);
