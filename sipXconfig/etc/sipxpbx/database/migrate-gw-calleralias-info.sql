create or replace function migrate_gcai() returns void as '\n
declare\n
  gw_rec record;\n
begin\n
 for gw_rec in select * from gateway gw where gw.enable_caller_id=true\n
  loop\n
    update gateway set default_caller_alias=gw_rec.caller_id where gateway_id=gw_rec.gateway_id;\n
  end loop;\n
end;\n
' language plpgsql;

select migrate_gcai();

alter table gateway drop column enable_caller_id;
alter table gateway drop column caller_id;