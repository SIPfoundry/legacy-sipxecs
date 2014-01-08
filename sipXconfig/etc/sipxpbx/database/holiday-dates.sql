-- need holidays to be saved as time intervals not just per day
alter table holiday_dates rename column date to start_date;
alter table holiday_dates add end_date timestamp;

