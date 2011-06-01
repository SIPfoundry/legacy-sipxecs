create user  cdrremote NOSUPERUSER NOCREATEDB  NOCREATEROLE;
grant select on cdrs to cdrremote;
grant select on view_cdrs to cdrremote;
