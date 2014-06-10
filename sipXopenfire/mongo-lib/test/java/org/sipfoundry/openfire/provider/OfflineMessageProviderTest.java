package org.sipfoundry.openfire.provider;

import static org.junit.Assert.assertEquals;

import java.util.Collection;

import org.dom4j.io.SAXReader;
import org.jivesoftware.openfire.OfflineMessage;
import org.jivesoftware.openfire.provider.OfflineMessageProvider;
import org.junit.After;
import org.junit.Test;
import org.xmpp.packet.Message;

import com.mongodb.BasicDBObject;

@SuppressWarnings("static-method")
public class OfflineMessageProviderTest extends BaseMongoTest {

    @After
    public void teardown() {
        getOpenfireDb().getCollection("ofOffline").remove(new BasicDBObject());
    }

    @Test
    public void testWrite() {
        OfflineMessageProvider provider = new MongoOfflineMessageProvider();

        provider.addMessage("gigel", 1, "gigel says 'hello world'");

        assertEquals(1, getOpenfireDb().getCollection("ofOffline").find().count());
    }

    @Test
    public void testRead() {
        String testMessage = "gigel says 'hello world'";
        OfflineMessageProvider provider = new MongoOfflineMessageProvider();
        SAXReader reader = new SAXReader();
        Message message = new Message();
        message.setBody(testMessage);

        provider.addMessage("gigel", 1, message.getElement().asXML());

        Collection<OfflineMessage> messagesFromDB = provider.getMessages("gigel", false, reader);

        assertEquals(1, messagesFromDB.size());

        OfflineMessage messageFromDB = messagesFromDB.iterator().next();

        assertEquals(testMessage, messageFromDB.getBody());
    }
}
