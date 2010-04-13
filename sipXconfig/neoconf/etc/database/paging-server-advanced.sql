-- alter paging server table
ALTER TABLE paging_server ADD COLUMN sip_trace_level CHARACTER VARYING(255);

UPDATE paging_server SET sip_trace_level='NONE';

ALTER TABLE paging_server ALTER COLUMN sip_trace_level SET NOT NULL;

