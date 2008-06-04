insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxRegistrarService');

insert into initialization_task (name) values ('initialize_sipx_registrar_service');