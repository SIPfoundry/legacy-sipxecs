package org.sipfoundry.commons.userdb;

import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.DB;

public class ValidUsersSpring extends ValidUsers {    
    private MongoTemplate m_imdbTemplate;

    public MongoTemplate getImdbTemplate() {
        return m_imdbTemplate;
    }

    public void setImdbTemplate(MongoTemplate imdbTemplate) {
        m_imdbTemplate = imdbTemplate;
    }
    
    public DB getImdb() {
        return m_imdbTemplate.getDb();
    }
}
