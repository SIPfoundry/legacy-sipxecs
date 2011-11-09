/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.commons.mongo;

import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;

import org.apache.axis.utils.StringUtils;
import org.springframework.beans.factory.annotation.Value;
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
    private String m_connectionString = "localhost:27017"; 
    
    public @Bean MongoFactoryBean mongo() throws UnknownHostException {
        MongoFactoryBean mongo = new MongoFactoryBean();
        MongoURI uri = new MongoURI(m_connectionString);
        List<ServerAddress> replicaSetSeeds = new ArrayList<ServerAddress>(uri.getHosts().size());
        for (String host : uri.getHosts()) {
            String[] hostPort = StringUtils.split(host, ':');
            replicaSetSeeds.add(new ServerAddress(hostPort[0], Integer.parseInt(hostPort[1])));            
        }
        mongo.setReplicaSetSeeds(replicaSetSeeds);
        mongo.setMongoOptions(uri.getOptions());
        
        return mongo;
    }

    public String getConnectionString() {
        return m_connectionString;
    }

    @Value("#{mongoClientProperties.SpringConnection}")
    public void setConnectionString(String connectionString) {
        m_connectionString = connectionString;
    }
}
