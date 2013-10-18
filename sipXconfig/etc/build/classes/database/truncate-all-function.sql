CREATE OR REPLACE FUNCTION truncate_all() RETURNS void AS $$
DECLARE
    row record;
    query   text;
BEGIN
    query := 'SELECT table_schema, table_name FROM information_schema.tables WHERE table_schema NOT IN(''pg_catalog'', ''information_schema'') AND table_type = ''BASE TABLE''';

    FOR row IN EXECUTE query LOOP
        EXECUTE 'TRUNCATE ' || quote_ident(row.table_schema) || '.' || quote_ident(row.table_name) || ' CASCADE;';
    END LOOP;

    RETURN;
END;
$$ language plpgsql;
