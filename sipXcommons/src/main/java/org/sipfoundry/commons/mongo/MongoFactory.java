/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.commons.mongo;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import org.apache.axis.utils.StringUtils;
import org.apache.commons.io.IOUtils;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.data.mongodb.core.MongoFactoryBean;

import com.mongodb.MongoURI;
import com.mongodb.ServerAddress;

/**
 * Creates a Mongo instance from a properties file 
 */
@Configuration
public class MongoFactory {
    private String m_configFile;
    private String m_connectionUrl; 
    
    public @Bean MongoFactoryBean mongo() throws UnknownHostException {
        readConfig();
        MongoFactoryBean mongo = new MongoFactoryBean();        
        // Go thru MongoURI for parsing only, do not get connection from the object itself
        MongoURI uri = new MongoURI(m_connectionUrl);
        List<ServerAddress> replicaSetSeeds = new ArrayList<ServerAddress>(uri.getHosts().size());
        for (String host : uri.getHosts()) {
            String[] hostPort = StringUtils.split(host, ':');
            replicaSetSeeds.add(new ServerAddress(hostPort[0], Integer.parseInt(hostPort[1])));            
        }
        mongo.setReplicaSetSeeds(replicaSetSeeds);
        mongo.setMongoOptions(uri.getOptions());
        
        return mongo;
    }
    
    String readConfig() {        
        if (m_connectionUrl == null) {
            Properties p = new Properties();
            FileInputStream in = null;
            try {
                in = new FileInputStream(m_configFile);
                p.load(in);
            } catch (IOException e) {
                throw new RuntimeException(e);
            } finally {
                IOUtils.closeQuietly(in);            
            }
            m_connectionUrl = p.getProperty("ConnectionUrl");
        }
        
        return m_connectionUrl;
    }
    
    public String getConnectionUrl() {
        return m_connectionUrl;
    }

    public void setConnectionUrl(String connectionString) {
        m_connectionUrl = connectionString;
    }

    public String getConfigFile() {
        return m_configFile;
    }

    public void setConfigFile(String configFile) {
        m_configFile = configFile;
    }
}
