/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.commons.mongo;

import java.io.File;
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
