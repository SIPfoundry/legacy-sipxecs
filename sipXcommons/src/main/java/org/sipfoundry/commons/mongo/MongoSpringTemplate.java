package org.sipfoundry.commons.mongo;

import org.springframework.beans.factory.annotation.Required;
import org.springframework.data.mongodb.MongoDbFactory;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.DB;


/**
 * Wrap template so database name can be set on template and not on factory. Factory
 * is singleton for all dbs to share Mongo object and reduce parsing of ini file 
 */
public class MongoSpringTemplate extends MongoTemplate {
    private String m_dbname;
    private MongoDbFactory m_factory;    
    
    public MongoSpringTemplate(MongoDbFactory factory) {
        super(factory);
        m_factory = factory;
    }
    
    @Override
    public DB getDb() {
        return m_factory.getDb(m_dbname);
    }

    public String getDbname() {
        return m_dbname;
    }

    @Required
    public void setDbname(String dbname) {
        m_dbname = dbname;
    }
}
