package org.sipfoundry.commons.mongo;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.Properties;

import org.apache.commons.io.IOUtils;

import com.mongodb.DB;
import com.mongodb.Mongo;
import com.mongodb.MongoException;
import com.mongodb.MongoURI;

public class MongoDbTemplate {
    int m_port = 27017;
    String m_server = null;
    String m_name;
    String m_configFile;
    Mongo m_mongo;
    
    public MongoDbTemplate() {        
    }
    
    public MongoDbTemplate(String dbname) {
        m_name = dbname;
    }

    /**
     * Load mongo uri connection object from properties file. Looks for entry name ConnectionUrl.
     * This does not actually connect to mongo servers, just creates uri objec that can be used
     * to connection
     * 
     * Example code:
     *   MongoDbTemplate db = new MongoDbTemplate("foo");
     *   db.setUri(MongoDbTemplate.parseFromFile("some/file"));
     *   db.getMongo();  // actually connects
     *  
     * Example file format:
     *   ConnectionUrl=mongodb://localhost:27117,localhost:27118/?slaveok=true
     */
    public static MongoURI parseFromFile(String file) {
        Properties p = new Properties();
        FileInputStream in = null;
        try {
            in = new FileInputStream(file);
            p.load(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(in);            
        }
        String url = p.getProperty("ConnectionUrl");
        MongoURI uri = new MongoURI(url);   
        return uri;
    }
    
    public Mongo getMongo() {
        if (m_mongo == null) {            
            if (m_name == null || (m_server == null && m_configFile == null)) {
                throw new IllegalStateException("db name not set on mongo db template");
            }
            try {
                if (m_configFile != null) {
                    MongoURI uri = MongoDbTemplate.parseFromFile(m_configFile);
                    m_mongo = new Mongo(uri);
                }
                else if (m_server != null) {
                    m_mongo = new Mongo(m_server, m_port);
                } else {
                    m_mongo = new Mongo();
                }
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

    public String getConfigFile() {
        return m_configFile;
    }

    public void setConfigFile(String configFile) {
        m_configFile = configFile;
    }    
}
