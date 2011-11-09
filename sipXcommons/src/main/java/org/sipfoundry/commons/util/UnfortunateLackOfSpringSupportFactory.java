package org.sipfoundry.commons.util;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.Properties;

import org.apache.commons.io.IOUtils;
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
    
    public synchronized static void initialize(String clientConfig) throws UnknownHostException {
        if (s_validUsers == null) {
            Properties p = new Properties();
            FileInputStream in = null;
            try {
                in = new FileInputStream(clientConfig);
                p.load(in);
            } catch (IOException e) {
                throw new RuntimeException(e);
            } finally {
                IOUtils.closeQuietly(in);            
            }
            String url = p.getProperty("mongoClientProperties.SpringConnection");
            MongoFactory factory = new MongoFactory();
            factory.setConnectionString(url);
            try {
                s_imdb = new MongoTemplate(factory.mongo().getObject(), "imdb");
                ValidUsers validUsers = new ValidUsers();
                validUsers.setImdb(s_imdb);
                // be sure this is the last successful thing you do to ensure
                // singleton is properly initialized 
                s_validUsers = validUsers; 
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
    }
    
    public static ValidUsers getValidUsers() {
        return s_validUsers;
    }
    
    public static MongoTemplate getImdb() {
        return s_imdb;        
    }
}
