alter table cron_schedule add column enabled bool;

update cron_schedule set enabled = false;
alter table cron_schedule alter column enabled set not null;
alter table cron_schedule alter column enabled set default false;
