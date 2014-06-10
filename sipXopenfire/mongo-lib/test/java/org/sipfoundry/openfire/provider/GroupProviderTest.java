package org.sipfoundry.openfire.provider;

import static org.junit.Assert.assertEquals;

import java.util.Collection;

import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupAlreadyExistsException;
import org.jivesoftware.openfire.provider.GroupProvider;
import org.junit.After;
import org.junit.Test;

import com.mongodb.BasicDBObject;
import com.mongodb.DBObject;

@SuppressWarnings("static-method")
public class GroupProviderTest extends BaseMongoTest {
    @After
    public void teardown() {
        getImdb().getCollection("entity").remove(new BasicDBObject());
    }

    @Test
    public void testCreateNonExistingGroup() throws GroupAlreadyExistsException {
        GroupProvider provider = new MongoGroupProviderAlt();
        Group g = provider.createGroup("testGrp");

        assertEquals(0L, getImdb().getCollection("entity").count());
        assertEquals(null, g);
    }

    @Test
    public void testCreateExistingGroup() throws GroupAlreadyExistsException {
        GroupProvider provider = new MongoGroupProviderAlt();

        insertGroup("testGrp");

        Group g = provider.createGroup("testGrp");

        assertEquals(1L, getImdb().getCollection("entity").count());
        assertEquals("testGrp", g.getName());
    }

    @Test
    public void testSearchWildcardPrefix() {
        GroupProvider provider = new MongoGroupProviderAlt();

        insertGroup("testGrp");
        insertGroup("additional");

        Collection<String> names = provider.search("*Grp");

        assertEquals(1L, names.size());
        for (String name : names) {
            assertEquals("testGrp", name);
        }
    }

    @Test
    public void testSearchWildcardSuffix() {
        GroupProvider provider = new MongoGroupProviderAlt();

        insertGroup("testGrp");
        insertGroup("additional");

        Collection<String> names = provider.search("test*");

        assertEquals(1L, names.size());
        for (String name : names) {
            assertEquals("testGrp", name);
        }
    }

    @Test
    public void testSearchWildcardMiddle() {
        GroupProvider provider = new MongoGroupProviderAlt();

        insertGroup("testGrp");
        insertGroup("additional");

        Collection<String> names = provider.search("te*rp");

        assertEquals(1L, names.size());
        for (String name : names) {
            assertEquals("testGrp", name);
        }
    }

    @Test
    public void testSearchWildcardAll() {
        GroupProvider provider = new MongoGroupProviderAlt();

        insertGroup("testGroup");
        insertGroup("additional");

        Collection<String> names = provider.search("*es*ro*");

        assertEquals(1L, names.size());
        for (String name : names) {
            assertEquals("testGroup", name);
        }
    }

    private static void insertGroup(String groupName) {
        DBObject grpObj = new BasicDBObject();

        grpObj.put("ent", "group");
        grpObj.put("uid", groupName);

        getImdb().getCollection("entity").insert(grpObj);
    }
}
