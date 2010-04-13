alter table personal_attendant alter override_language set default 'false';
update personal_attendant set override_language = 'false' where override_language = null;
alter table personal_attendant alter override_language set not null;
