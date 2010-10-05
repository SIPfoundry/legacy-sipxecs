create or replace function migrate_sbcs() returns void as '\n
declare\n
  sbc_device_rec record;\n
begin\n
  CREATE TEMP TABLE sbc_devices AS SELECT address, port, sbc_device_id, bean_id FROM sbc_device;\n
\n
  for sbc_device_rec in select * from sbc_devices\n
  loop\n
    UPDATE sbc\n
    SET address_actual=sbc_device_rec.address, port=sbc_device_rec.port\n
    WHERE sbc.sbc_device_id=sbc_device_rec.sbc_device_id;\n
\n
    UPDATE gateway\n
    SET outbound_address=sbc_device_rec.address, outbound_port=sbc_device_rec.port, use_sipxbridge=''f''\n
    WHERE gateway.sbc_device_id=sbc_device_rec.sbc_device_id AND sbc_device_rec.bean_id != ''sbcSipXbridge'';\n

    UPDATE gateway\n
    SET outbound_address=sbc_device_rec.address, outbound_port=sbc_device_rec.port, use_sipxbridge=''t''\n
    WHERE gateway.sbc_device_id=sbc_device_rec.sbc_device_id AND sbc_device_rec.bean_id = ''sbcSipXbridge'';\n


  end loop;\n

  -- set default value for port column in sbc if no sbc_device is associated with it
  UPDATE sbc\n
  SET port=5060\n
  WHERE sbc_device_id IS NULL;\n

  -- set default value for outbound_port column in gateway if no device is associated with it
  UPDATE gateway\n
  SET outbound_port=5060, use_sipxbridge=''f''\n
  WHERE sbc_device_id IS NULL;\n
end;\n
' language plpgsql;

select migrate_sbcs();

DROP TABLE sbc_devices;
