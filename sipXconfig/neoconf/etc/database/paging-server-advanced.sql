-- alter paging server table
alter table paging_server add column sip_trace_level character varying(255) not null;