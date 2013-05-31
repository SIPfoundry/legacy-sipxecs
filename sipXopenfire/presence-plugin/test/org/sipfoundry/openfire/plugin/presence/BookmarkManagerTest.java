package org.sipfoundry.openfire.plugin.presence;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.util.List;

import org.dom4j.DocumentHelper;
import org.dom4j.Element;
import org.dom4j.Node;
import org.dom4j.XPath;
import org.dom4j.xpath.DefaultXPath;
import org.junit.Test;

public class BookmarkManagerTest {

    @SuppressWarnings("static-method")
    @Test
    public void testPathMatching() {
        Element doc = buildInitialDoc();
        XPath roomPath = new DefaultXPath("conference[@name='200room'|jid='200room@conference.xmpptest.ezuce.ro']");

        Node match = roomPath.selectSingleNode(doc);
        assertNotNull(match);
    }

    @SuppressWarnings("static-method")
    @Test
    public void testRoomAdd() {
        Element doc = buildInitialDoc();
        Element newRoom = buildConfElement("201room");
        XPath roomPath = new DefaultXPath("conference");

        doc.add(newRoom);

        assertEquals(2, roomPath.selectNodes(doc).size());
    }

    @SuppressWarnings({
        "static-method", "unchecked"
    })
    @Test
    public void testRoomDelete() {
        Element doc = buildInitialDoc();
        XPath roomPath = new DefaultXPath("conference[@jid='200room@conference.xmpptest.ezuce.ro']");
        List<Node> matches = roomPath.selectNodes(doc);

        for (Node match : matches) {
            doc.remove(match);
        }
        matches = roomPath.selectNodes(doc);

        assertEquals(0, matches.size());
    }

    private static Element buildConfElement(String roomName) {
        Element elem = DocumentHelper.createElement("conference");

        elem.add(DocumentHelper.createAttribute(elem, "name", roomName));
        elem.add(DocumentHelper.createAttribute(elem, "autojoin", "false"));
        elem.add(DocumentHelper.createAttribute(elem, "jid", roomName + "@conference.xmpptest.ezuce.ro"));

        return elem;
    }

    private static Element buildStorageElement() {
        Element elem = DocumentHelper.createElement("storage");

        elem.add(DocumentHelper.createNamespace("", "storage:bookmarks"));

        return elem;
    }

    private static Element buildInitialDoc() {
        Element confElem = buildConfElement("200room");
        Element storElem = buildStorageElement();

        storElem.add(confElem);

        return storElem;
    }

}
