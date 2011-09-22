package org.sipfoundry.commons.util;

import org.sipfoundry.commons.mongo.MongoDbTemplate;
import org.sipfoundry.commons.userdb.ValidUsers;

public class UnfortunateLackOfSpringSupportFactory {
    
    private static class SingletonHolder {
        final private static ValidUsers s_validUsers = new ValidUsers();
        final private static MongoDbTemplate s_imdb = new MongoDbTemplate();   
        static {
            s_validUsers.setImdb(s_imdb);
        }
    }
    
    public static ValidUsers getValidUsers() {
        return SingletonHolder.s_validUsers;
    }
    
    public static MongoDbTemplate getImdb() {
        return SingletonHolder.s_imdb;        
    }
}
