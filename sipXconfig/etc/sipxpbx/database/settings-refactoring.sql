-- strip root path prefix
update setting_value set path = substring(path, 2);