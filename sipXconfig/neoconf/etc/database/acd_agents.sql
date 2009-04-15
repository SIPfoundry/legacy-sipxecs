ALTER TABLE acd_agent 
    ADD COLUMN acd_server_id int4;
    
-- all existing agents will belong to the first server    
UPDATE acd_agent SET acd_server_id = (SELECT acd_server_id FROM acd_server ORDER BY acd_server_id LIMIT 1);    
    
ALTER TABLE acd_agent ALTER COLUMN acd_server_id SET NOT NULL;
    
ALTER TABLE acd_agent 
    ADD CONSTRAINT fk_acd_agent_server 
    FOREIGN KEY (acd_server_id) 
    REFERENCES acd_server;
    
    
CREATE INDEX index_acd_agent_user_server ON acd_agent(acd_agent_id, user_id, acd_server_id);
