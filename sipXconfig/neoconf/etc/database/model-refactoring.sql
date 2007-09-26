-- FOR UPGRADE ONLY
update phone set model_id = 'cisco' || model_id where bean_id = 'ciscoAta';
update phone set model_id = 'cisco' || model_id where bean_id = 'ciscoIp';
update phone set model_id = 'gs' || model_id where bean_id = 'grandstream';
update phone set model_id = 'kphoneStandard' where bean_id = 'kphone';
update phone set model_id = 'polycom' || model_id where bean_id = 'polycom';
update phone set model_id = 'snom' || model_id where bean_id = 'snom';
update phone set model_id = 'hitachi' || model_id where bean_id = 'hitachi';
update phone set model_id = 'acmePhoneStandard', bean_id = 'acmePhone' where bean_id = 'unmanagedPhone' or bean_id = 'acmePhone';
update gateway set model_id = 'audiocodesTP260_' || model_id where bean_id = 'gwAudiocodesTp260';
update gateway set model_id = 'audiocodesMP_' || model_id where bean_id = 'gwAudiocodesMediant';
update gateway set model_id = 'genericGatewayStandard' where bean_id = 'gwGeneric';
update gateway set model_id = 'sipTrunkStandard' where bean_id = 'gwSipTrunk';


-- FOR SCHEMA
alter table phone
	alter column model_id
        set not null;

alter table gateway
	alter column model_id
        set not null;
