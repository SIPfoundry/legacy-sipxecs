package org.sipfoundry.openfire.provider;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import org.jivesoftware.openfire.provider.UIDProvider;
import org.junit.After;
import org.junit.Test;

import com.mongodb.BasicDBObject;

@SuppressWarnings("static-method")
public class UIDProviderTest extends BaseMongoTest {

    @After
    public void teardown() {
        getOpenfireDb().getCollection("ofId").remove(new BasicDBObject());
    }

    @Test
    public void testGetBlock() {
        UIDProvider provider = new MongoUIDProvider();

        long[] ids = provider.getNextBlock(1, 5);

        assertNotNull(ids);
        assertEquals(2, ids.length);
        assertEquals(1, ids[0]); // starting id for a type that's not in the DB yet
        assertEquals(6, ids[1]); // next starting id
    }

    @Test
    public void testGetBlockAgain() {
        UIDProvider provider = new MongoUIDProvider();

        long[] ids = provider.getNextBlock(1, 3);
        ids = provider.getNextBlock(1, 15);

        assertEquals(4, ids[0]); // starts where the first request ended
        assertEquals(19, ids[1]); // next starting id
    }

    @Test
    public void testGetBlockHandleLong() {
        UIDProvider provider = new MongoUIDProvider();

        long[] ids = provider.getNextBlock(1, 7);
        ids = provider.getNextBlock(1, Integer.MAX_VALUE); // that'll bring us into long territory

        assertEquals(8, ids[0]); // starts where the first request ended
        assertEquals(Integer.MAX_VALUE + 8L, ids[1]); // next starting id
    }

}
