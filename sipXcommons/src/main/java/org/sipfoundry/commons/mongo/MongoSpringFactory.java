/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.commons.mongo;

import java.net.UnknownHostException;

import org.springframework.dao.DataAccessException;
import org.springframework.data.mongodb.MongoDbFactory;
import org.springframework.data.mongodb.core.SimpleMongoDbFactory;

import com.mongodb.DB;
import com.mongodb.Mongo;
import com.mongodb.MongoException;
import com.mongodb.MongoURI;

/**
 * Creates a Mongo instance from a properties file for spring based projects
 * 
 */
public class MongoSpringFactory implements MongoDbFactory {
    private SimpleMongoDbFactory m_delegate;
    private String m_configFile;
    private String m_connectionUrl;

    @Override
    public DB getDb() throws DataAccessException {
        return getDelegate().getDb();
    }

    @Override
    public DB getDb(String name) throws DataAccessException {        
        return getDelegate().getDb(name);
    }
    
    private MongoDbFactory getDelegate() {
        if (m_delegate == null) {
            if (m_connectionUrl == null) {
                m_connectionUrl = MongoFactory.readConfig(m_configFile);
            }
            MongoURI uri = new MongoURI(m_connectionUrl);        
            try {
                m_delegate = new SimpleMongoDbFactory(new Mongo(uri), "notused");
            } catch (MongoException e) {
                throw new MongoConfigException(e);
            } catch (UnknownHostException e) {
                throw new MongoConfigException(e);
            }            
        }
        return m_delegate;
    }
    
    static class MongoConfigException extends DataAccessException {
        public MongoConfigException(Throwable cause) {
            super("Cannot get mongo connection", cause);
        }        
    }
    
    public void setConnectionUrl(String url)   {
        m_connectionUrl = url;
    }

    public void setConfigFile(String configFile) {
        m_configFile = configFile;
    }
}
