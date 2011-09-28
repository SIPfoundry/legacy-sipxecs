package org.sipfoundry.commons.mongo;

import java.net.UnknownHostException;

import com.mongodb.DB;
import com.mongodb.Mongo;
import com.mongodb.MongoException;

public class MongoDbTemplate {
    int m_port = 27017;
    String m_server = "localhost";
    String m_name;
    Mongo m_mongo;
    
    public Mongo getMongo() {
        if (m_mongo == null) {
            if (m_name == null) {
                throw new IllegalStateException("db name not set on mongo db template");
            }
            try {
                m_mongo = new Mongo(m_server, m_port);
            } catch (UnknownHostException e) {
                String msg = String.format("Bad server name '%s' port %i for mongo connection", m_server, m_port);
                throw new RuntimeException(msg, e);
            } catch (MongoException e) {
                String msg = String.format("Error connecting to mongo server '%s' port %i ", m_server, m_port);
                throw new RuntimeException(msg, e);
            }
        }
        
        return m_mongo;
    }

    public int getPort() {
        return m_port;
    }
    public void setPort(int port) {
        m_port = port;
    }
    public String getServer() {
        return m_server;
    }
    public void setServer(String server) {
        m_server = server;
    }
    
    public DB getDb() {
        return getMongo().getDB(m_name);        
    }
    
    public void drop() {
        getMongo().dropDatabase(m_name);
    }
    
    public String getName() {
        return m_name;
    }
    
    public void setName(String dbname) {
        m_name = dbname;
    }    
}
