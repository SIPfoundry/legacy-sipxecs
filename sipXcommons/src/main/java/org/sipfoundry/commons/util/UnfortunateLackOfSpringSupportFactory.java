package org.sipfoundry.commons.util;

import org.sipfoundry.commons.mongo.MongoFactory;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.springframework.data.mongodb.core.MongoTemplate;

/**
 * Connection factory to mongo.  Need to call this before using
 *   UnfortunateLackOfSpringSupportFactory.initialize("/file/to/client/config");
 * File is probably
 *   @SIPX_CONFDIR@/mongo-client.ini
 */
public class UnfortunateLackOfSpringSupportFactory {
    private static ValidUsers s_validUsers;
    private static MongoTemplate s_imdb;   
    
    public synchronized static void initialize(String clientConfig) {
        if (s_validUsers == null) {
            
            // useful in unit tests to direct operations to imdb_TEST
            String imdbNs = System.getProperty("mongo_ns", "imdb");
            
            MongoFactory factory = new MongoFactory();
            factory.setConfigFile(clientConfig);
            try {
                s_imdb = new MongoTemplate(factory.mongo().getObject(), imdbNs);
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
            ValidUsers validUsers = new ValidUsers();
            validUsers.setImdb(s_imdb);
            // be sure this is the last successful thing you do to ensure
            // singleton is properly initialized 
            s_validUsers = validUsers; 
        }
    }
    
    public static ValidUsers getValidUsers() {
        return s_validUsers;
    }
    
    public static MongoTemplate getImdb() {
        return s_imdb;        
    }
}
