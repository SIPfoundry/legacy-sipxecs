create or replace function migrate_sbcs() returns void as $$
declare
  sbc_device_rec record;
begin
  CREATE TEMP TABLE sbc_devices AS SELECT address, port, sbc_device_id, bean_id FROM sbc_device;

  for sbc_device_rec in select * from sbc_devices
  loop
    UPDATE sbc
    SET address_actual=sbc_device_rec.address, port=sbc_device_rec.port
    WHERE sbc.sbc_device_id=sbc_device_rec.sbc_device_id;

    UPDATE gateway
    SET outbound_address=sbc_device_rec.address, outbound_port=sbc_device_rec.port, use_sipxbridge='f'
    WHERE gateway.sbc_device_id=sbc_device_rec.sbc_device_id AND sbc_device_rec.bean_id != 'sbcSipXbridge';

    UPDATE gateway
    SET outbound_address=sbc_device_rec.address, outbound_port=sbc_device_rec.port, use_sipxbridge='t'
    WHERE gateway.sbc_device_id=sbc_device_rec.sbc_device_id AND sbc_device_rec.bean_id = 'sbcSipXbridge';


  end loop;

  -- set default value for port column in sbc if no sbc_device is associated with it
  UPDATE sbc
  SET port=5060
  WHERE sbc_device_id IS NULL;

  -- set default value for outbound_port column in gateway if no device is associated with it
  UPDATE gateway
  SET outbound_port=5060, use_sipxbridge='f'
  WHERE sbc_device_id IS NULL;
end;
$$ language plpgsql;

select migrate_sbcs();

DROP TABLE sbc_devices;
