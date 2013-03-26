/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.rls;

import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.insertJson;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.commons.userdb.ValidUsersSpring;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.rls.ResourceLists;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.SimpleMongoDbFactory;

import com.mongodb.DBCollection;
import com.mongodb.Mongo;

public class ResourceListsTest extends XMLTestCase {
    public final static String DOMAIN = "example.org";
    private CoreContext m_coreContext;
    private MongoTemplate m_db; 
    private ResourceLists m_rl;

    @Override
    protected void setUp() throws Exception {
        m_db = new MongoTemplate(new SimpleMongoDbFactory(new Mongo(), "test"));
        m_coreContext = createMock(CoreContext.class);
        m_coreContext.getDomainName();
        expectLastCall().andReturn(DOMAIN).anyTimes();
        replay(m_coreContext);
        m_db.getDb().dropDatabase();
        m_rl = new ResourceLists();
        ValidUsersSpring vu = new ValidUsersSpring();
        vu.setImdbTemplate(m_db);
        m_rl.setValidUsers(vu);
        m_rl.setCoreContext(m_coreContext);
    }

    private DBCollection getEntityCollection() {
        return m_db.getDb().getCollection(MongoConstants.ENTITY_COLLECTION);
    }

    public void testGenerate() throws Exception {
        String json1 = "{ \"_id\" : \"User1\", \"uid\" : \"user_a\", \"imenbld\" : false, \"ent\": \"user\", " + "\""
                + MongoConstants.SPEEDDIAL + "\": { \"" + MongoConstants.USER + "\" : \"~~rl~F~user_a\", \""
                + MongoConstants.USER_CONS + "\" : \"~~rl~C~user_a\", \"" + MongoConstants.BUTTONS + "\" : [ "
                + "{\"" + MongoConstants.URI + "\" : \"sip:102@example.org\",\"" + MongoConstants.NAME
                + "\" : \"beta\"}," + "{\"" + MongoConstants.URI + "\" : \"sip:104@sipfoundry.org\",\""
                + MongoConstants.NAME + "\" : \"gamma\"}" + "]}, \"prm\" : [\"subscribe-to-presence\"" + "]}";

        String json2 = "{ \"_id\" : \"User2\", \"uid\" : \"user_c\", \"imenbld\" : false, \"ent\": \"user\"," + "\""
                + MongoConstants.SPEEDDIAL + "\" : { \"" + MongoConstants.USER + "\" : \"~~rl~F~user_c\", \""
                + MongoConstants.USER_CONS + "\" : \"~~rl~C~user_c\", \"" + MongoConstants.BUTTONS + "\" : [ "
                + "{\"" + MongoConstants.URI + "\" : \"sip:100@example.org\",\"" + MongoConstants.NAME
                + "\" : \"delta\"}" + "]}, \"prm\" : [\"subscribe-to-presence\"" + "]}";

        String json3 = "{ \"_id\" : \"User3\", \"uid\" : \"user_name_0\", \"imenbld\" : true, \"ent\": \"user\"}";

        String json4 = "{ \"_id\" : \"User4\", \"uid\" : \"user_name_1\", \"imenbld\" : true, ent: \"user\"}";
        insertJson(getEntityCollection(), json1, json2, json3, json4);
        Thread.sleep(1000);// sleep 1 second. I get inconsistent results when running the tests,
                           // as if mongo does not pick up quickly

        Document doc = m_rl.getDocument(true);        
        String generatedXml = TestHelper.asString(doc);
        InputStream referenceXml = getClass().getResourceAsStream("resource-lists.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));
        doc = m_rl.getDocument(false);        
        generatedXml = TestHelper.asString(doc);
        referenceXml = getClass().getResourceAsStream("resource-lists-no-xmpp.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));
    }

    /**
     * No matter if the user does not have blf permission, we should keep generating what speeddials we have
     * The BLF permission turning off only restricts the user to change his blf permission from user-portal
     * @throws Exception
     */
    public void testGenerateNoBlfPerm() throws Exception {
        String json3 = "{ \"_id\" : \"User3\", \"uid\" : \"user_name_0\", \"imenbld\" : false, \"ent\": \"user\"}";

        String json2 = "{ \"_id\" : \"User2\", \"uid\" : \"user_c\", \"imenbld\" : false, \"ent\": \"user\", " + "\""
                + MongoConstants.SPEEDDIAL + "\" : { \"" + MongoConstants.USER + "\" : \"~~rl~F~user_c\", \""
                + MongoConstants.USER_CONS + "\" : \"~~rl~C~user_c\", \"" + MongoConstants.BUTTONS + "\" : [ "
                + "{\"" + MongoConstants.URI + "\" : \"sip:100@example.org\",\"" + MongoConstants.NAME
                + "\" : \"delta\"}" + "]}, \"prm\" : [\"Mobile\"" + "]}";

        insertJson(getEntityCollection(), json3, json2);
        Thread.sleep(1000);

        Document doc = m_rl.getDocument(true);        
        String fileContent = TestHelper.asString(doc);
        InputStream referenceXml = getClass().getResourceAsStream("resource-lists-noblfperm.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(fileContent));
    }
}
