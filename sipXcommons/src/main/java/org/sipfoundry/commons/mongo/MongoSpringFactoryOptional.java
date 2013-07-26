package org.sipfoundry.commons.mongo;

import java.io.File;

import org.springframework.dao.DataAccessException;
import org.springframework.data.mongodb.MongoDbFactory;

import com.mongodb.DB;

/**
 * Like MongoSpringFactory but config file might not exist.  If it doesn't then
 * this instance returns null for Db.  I created a separate class for this behavior
 * because under many systems the config file is not optional and you'd expect failfast
 * behavior there.
 * 
 * Ultimately sipxconfig uses this class to determine if system has a local
 * database defined and if it does, use it.
 */
public class MongoSpringFactoryOptional implements MongoDbFactory {
    private MongoSpringFactory m_delegate;

    @Override
    public DB getDb() throws DataAccessException {
        return m_delegate != null ? m_delegate.getDb() : null;
    }

    @Override
    public DB getDb(String dbname) throws DataAccessException {
        return m_delegate != null ? m_delegate.getDb(dbname) : null;
    }
    
    public void setConfigFile(String configFile) {
        if (new File(configFile).exists()) {
            m_delegate = new MongoSpringFactory();
            m_delegate.setConfigFile(configFile);
        }
    }    
}
