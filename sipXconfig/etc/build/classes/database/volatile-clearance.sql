ALTER TABLE attendant_dialing_rule DROP CONSTRAINT fk_attendant_dialing_rule_dialing_rule;
ALTER TABLE custom_dialing_rule_permission DROP CONSTRAINT FK8F3EE457454433A3;
ALTER TABLE dial_pattern DROP CONSTRAINT FK8D4D2DC1454433A3;
ALTER TABLE custom_dialing_rule DROP CONSTRAINT FKB189AEB7454433A3;
ALTER TABLE emergency_dialing_rule DROP CONSTRAINT FK7EAEE897444E2DC3;
ALTER TABLE internal_dialing_rule DROP CONSTRAINT FK5D102EEBDE4556EF;
ALTER TABLE international_dialing_rule DROP CONSTRAINT FKE5D682BA7DD83CC0;
ALTER TABLE local_dialing_rule DROP CONSTRAINT FK365020FD76B0539D;
ALTER TABLE long_distance_dialing_rule DROP CONSTRAINT FKA10B67307DD83CC0;
ALTER TABLE dialing_rule_gateway DROP CONSTRAINT FK65E824AE38F854F6;
ALTER TABLE dialing_rule_gateway DROP CONSTRAINT FK65E824AEF6075471;


ALTER TABLE attendant_dialing_rule ADD CONSTRAINT fk_attendant_dialing_rule_dialing_rule_cascade  foreign key (attendant_dialing_rule_id) references dialing_rule ON UPDATE NO ACTION ON DELETE CASCADE;
ALTER TABLE custom_dialing_rule_permission ADD CONSTRAINT FK8F3EE457454433A3_cascade foreign key (custom_dialing_rule_id) references custom_dialing_rule ON UPDATE NO ACTION ON DELETE CASCADE;
ALTER TABLE dial_pattern ADD CONSTRAINT FK8D4D2DC1454433A3_cascade foreign key (custom_dialing_rule_id) references custom_dialing_rule
ON UPDATE NO ACTION ON DELETE CASCADE;
ALTER TABLE custom_dialing_rule ADD CONSTRAINT FKB189AEB7454433A3_cascade foreign key (custom_dialing_rule_id) references dialing_rule ON UPDATE NO ACTION ON DELETE CASCADE;
ALTER TABLE emergency_dialing_rule ADD CONSTRAINT FK7EAEE897444E2DC3_cascade foreign key (emergency_dialing_rule_id) references dialing_rule ON UPDATE NO ACTION ON DELETE CASCADE;
ALTER TABLE internal_dialing_rule ADD CONSTRAINT FK5D102EEBDE4556EF_cascade foreign key (internal_dialing_rule_id) references dialing_rule
ON UPDATE NO ACTION ON DELETE CASCADE;
ALTER TABLE international_dialing_rule ADD CONSTRAINT FKE5D682BA7DD83CC0_cascade foreign key (international_dialing_rule_id) references dialing_rule ON UPDATE NO ACTION ON DELETE CASCADE;
ALTER TABLE local_dialing_rule ADD CONSTRAINT FK365020FD76B0539D_cascade foreign key (local_dialing_rule_id) references dialing_rule
ON UPDATE NO ACTION ON DELETE CASCADE;
ALTER TABLE long_distance_dialing_rule ADD CONSTRAINT FKA10B67307DD83CC0_cascade foreign key (international_dialing_rule_id) references dialing_rule ON UPDATE NO ACTION ON DELETE CASCADE;
ALTER TABLE dialing_rule_gateway ADD CONSTRAINT referenceGatewayCascade foreign key (gateway_id) references gateway ON UPDATE NO ACTION ON DELETE CASCADE;
ALTER TABLE dialing_rule_gateway ADD CONSTRAINT referenceRuleCascade foreign key (dialing_rule_id) references dialing_rule ON UPDATE NO ACTION ON DELETE CASCADE;

DELETE FROM dialing_rule where dialing_rule_id IN (SELECT dialing_rule_id FROM dialing_rule_gateway WHERE gateway_id IN (SELECT gateway_id from gateway where model_id = 'genericVolatileGatewayStandard'));
DELETE FROM gateway where model_id = 'genericVolatileGatewayStandard';

ALTER TABLE dialing_rule_gateway DROP CONSTRAINT referenceGatewayCascade;
ALTER TABLE dialing_rule_gateway DROP CONSTRAINT referenceRuleCascade;
ALTER TABLE attendant_dialing_rule DROP CONSTRAINT fk_attendant_dialing_rule_dialing_rule_cascade;
ALTER TABLE custom_dialing_rule_permission DROP CONSTRAINT FK8F3EE457454433A3_cascade;
ALTER TABLE dial_pattern DROP CONSTRAINT FK8D4D2DC1454433A3_cascade;
ALTER TABLE custom_dialing_rule DROP CONSTRAINT FKB189AEB7454433A3_cascade;
ALTER TABLE emergency_dialing_rule DROP CONSTRAINT FK7EAEE897444E2DC3_cascade;
ALTER TABLE internal_dialing_rule DROP CONSTRAINT FK5D102EEBDE4556EF_cascade;
ALTER TABLE international_dialing_rule DROP CONSTRAINT FKE5D682BA7DD83CC0_cascade;
ALTER TABLE local_dialing_rule DROP CONSTRAINT FK365020FD76B0539D_cascade;
ALTER TABLE long_distance_dialing_rule DROP CONSTRAINT FKA10B67307DD83CC0_cascade;

ALTER TABLE attendant_dialing_rule ADD CONSTRAINT fk_attendant_dialing_rule_dialing_rule foreign key (attendant_dialing_rule_id) references dialing_rule;
ALTER TABLE custom_dialing_rule_permission ADD CONSTRAINT FK8F3EE457454433A3 foreign key (custom_dialing_rule_id) references custom_dialing_rule;
ALTER TABLE dial_pattern ADD CONSTRAINT FK8D4D2DC1454433A3 foreign key (custom_dialing_rule_id) references custom_dialing_rule;
ALTER TABLE custom_dialing_rule ADD CONSTRAINT FKB189AEB7454433A3 foreign key (custom_dialing_rule_id) references dialing_rule;
ALTER TABLE emergency_dialing_rule ADD CONSTRAINT FK7EAEE897444E2DC3 foreign key (emergency_dialing_rule_id) references dialing_rule;
ALTER TABLE internal_dialing_rule ADD CONSTRAINT FK5D102EEBDE4556EF foreign key (internal_dialing_rule_id) references dialing_rule;
ALTER TABLE international_dialing_rule ADD CONSTRAINT FKE5D682BA7DD83CC0 foreign key (international_dialing_rule_id) references dialing_rule;
ALTER TABLE local_dialing_rule ADD CONSTRAINT FK365020FD76B0539D foreign key (local_dialing_rule_id) references dialing_rule;
ALTER TABLE long_distance_dialing_rule ADD CONSTRAINT FKA10B67307DD83CC0 foreign key (international_dialing_rule_id) references dialing_rule;
ALTER TABLE dialing_rule_gateway ADD CONSTRAINT FK65E824AE38F854F6 foreign key (gateway_id) references gateway;
ALTER TABLE dialing_rule_gateway ADD CONSTRAINT FK65E824AEF6075471 foreign key (dialing_rule_id) references dialing_rule;

insert into initialization_task (name) values ('cleanup-dial-plans');