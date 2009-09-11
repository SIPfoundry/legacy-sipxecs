-- switch internal rules to use freeswitchMediaServer
update internal_dialing_rule
  set media_server_type = 'freeswitchMediaServer'
  where media_server_type = 'sipXMediaServer';

-- clean-up all referenced to sipxMediaService
delete from setting_value
  using sipx_service
  where sipx_service.bean_id = 'sipxMediaService'
    and setting_value.value_storage_id = sipx_service.value_storage_id;

delete from value_storage
  using sipx_service
  where sipx_service.bean_id = 'sipxMediaService'
    and value_storage.value_storage_id = sipx_service.value_storage_id;

delete from location_specific_service
  using sipx_service
  where sipx_service.bean_id = 'sipxMediaService'
    and location_specific_service.sipx_service_id = sipx_service.sipx_service_id;

delete from sipx_service where bean_id = 'sipxMediaService';
