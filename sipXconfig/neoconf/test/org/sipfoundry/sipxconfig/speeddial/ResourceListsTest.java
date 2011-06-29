/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.speeddial;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;

import org.custommonkey.xmlunit.XMLTestCase;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.MongoTestCaseHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;

import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;

public class ResourceListsTest extends XMLTestCase {
    public final static String DOMAIN = "example.org";
    private CoreContext m_coreContext;
    private DBCollection m_collection;

    @Override
    protected void setUp() throws Exception {
        m_coreContext = createMock(CoreContext.class);
        m_coreContext.getDomainName();
        expectLastCall().andReturn(DOMAIN).anyTimes();
        replay(m_coreContext);
        MongoTestCaseHelper.dropDb("imdb");
        m_collection = MongoTestCaseHelper.initMongo("imdb", "entity");
    }

    public void testGenerate() throws Exception {
        String json1 = "{ \"_id\" : \"User1\", \"uid\" : \"user_a\", \"imenbld\" : \"false\", " + "\""
                + MongoConstants.SPEEDDIAL + "\": { \"" + MongoConstants.USER + "\" : \"~~rl~F~user_a\", \""
                + MongoConstants.USER_CONS + "\" : \"~~rl~C~user_a\", \"" + MongoConstants.BUTTONS + "\" : [ "
                + "{\"" + MongoConstants.URI + "\" : \"sip:102@example.org\",\"" + MongoConstants.NAME
                + "\" : \"beta\"}," + "{\"" + MongoConstants.URI + "\" : \"sip:104@sipfoundry.org\",\""
                + MongoConstants.NAME + "\" : \"gamma\"}" + "]}, \"prm\" : [\"subscribe-to-presence\"" + "]}";

        String json2 = "{ \"_id\" : \"User2\", \"uid\" : \"user_c\", \"imenbld\" : \"false\", " + "\""
                + MongoConstants.SPEEDDIAL + "\" : { \"" + MongoConstants.USER + "\" : \"~~rl~F~user_c\", \""
                + MongoConstants.USER_CONS + "\" : \"~~rl~C~user_c\", \"" + MongoConstants.BUTTONS + "\" : [ "
                + "{\"" + MongoConstants.URI + "\" : \"sip:100@example.org\",\"" + MongoConstants.NAME
                + "\" : \"delta\"}" + "]}, \"prm\" : [\"subscribe-to-presence\"" + "]}";

        String json3 = "{ \"_id\" : \"User3\", \"uid\" : \"user_name_0\", \"imenbld\" : \"true\"}";

        String json4 = "{ \"_id\" : \"User4\", \"uid\" : \"user_name_1\", \"imenbld\" : \"true\"}";
        MongoTestCaseHelper.insertJson(json1, json2, json3, json4);
        Thread.sleep(1000);// sleep 1 second. I get inconsistent results when running the tests,
                           // as if mongo does not pick up quickly
        ResourceLists rl = new ResourceLists();
        rl.setCoreContext(m_coreContext);

        String generatedXml = getFileContent(rl, null);
        InputStream referenceXml = getClass().getResourceAsStream("resource-lists.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));
    }

    public void testGenerateEmpty() throws Exception {
        String json3 = "{ \"_id\" : \"User3\", \"uid\" : \"user_name_0\", \"imenbld\" : \"false\"}";

        String json2 = "{ \"_id\" : \"User2\", \"uid\" : \"user_c\", \"imenbld\" : \"false\", " + "\""
                + MongoConstants.SPEEDDIAL + "\" : { \"" + MongoConstants.USER + "\" : \"~~rl~F~user_c\", \""
                + MongoConstants.USER_CONS + "\" : \"~~rl~C~user_c\", \"" + MongoConstants.BUTTONS + "\" : [ "
                + "{\"" + MongoConstants.URI + "\" : \"sip:100@example.org\",\"" + MongoConstants.NAME
                + "\" : \"delta\"}" + "]}, \"prm\" : [\"Mobile\"" + "]}";

        MongoTestCaseHelper.insertJson(json3, json2);
        Thread.sleep(1000);
        ResourceLists rl = new ResourceLists();
        rl.setCoreContext(m_coreContext);

        String fileContent = getFileContent(rl, null);
        assertXMLEqual("<lists xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/resource-lists-00-01\"/>",
                fileContent);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        MongoTestCaseHelper.dropDb("imdb");
    }
}
