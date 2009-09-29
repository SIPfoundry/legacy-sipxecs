/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.util.List;

import junit.framework.TestCase;

import org.dom4j.DocumentFactory;
import org.dom4j.Element;

public class FullTransformTest extends TestCase {

    public void testAddChildren() {
        FullTransform ft = new FullTransform();
        ft.setUser("testuser");
        ft.setHost("testhost");
        ft.setFieldParams("fp1", "fp2");
        ft.setHeaderParams(new String[] {
            "hp1", "hp2"
        });
        ft.setUrlParams("url1");

        Element element = DocumentFactory.getInstance().createElement("test");
        ft.addChildren(element);

        // check the contents
        assertEquals("testhost", element.valueOf("host"));
        assertEquals("testuser", element.valueOf("user"));
        assertEquals("fp1", element.valueOf("fieldparams[1]"));
        assertEquals("fp2", element.valueOf("fieldparams[2]"));
        assertEquals("hp1", element.valueOf("headerparams[1]"));
        assertEquals("hp2", element.valueOf("headerparams[2]"));
        assertEquals("url1", element.valueOf("urlparams[1]"));

        // check the order
        String[] expected = {
            "user", "host", "urlparams", "headerparams", "headerparams", "fieldparams",
            "fieldparams"
        };
        List<Element> actual = element.elements();
        assertEquals(expected.length, actual.size());
        for (int i = 0; i < expected.length; i++) {
            assertEquals("Comparing element " + i, expected[i], actual.get(i).getName());
        }
    }

    public void testAddFieldParams() {
        FullTransform ft = new FullTransform();
        ft.addFieldParams("fp1", "fp2");

        String[] fp = ft.getFieldParams();
        assertEquals(2, fp.length);
        assertEquals("fp1", fp[0]);
        assertEquals("fp2", fp[1]);

        ft.addFieldParams("fp3");
        fp = ft.getFieldParams();
        assertEquals(3, fp.length);
        assertEquals("fp1", fp[0]);
        assertEquals("fp2", fp[1]);
        assertEquals("fp3", fp[2]);
    }

    public void testAddChildrenEmpty() {
        FullTransform ft = new FullTransform();

        Element element = DocumentFactory.getInstance().createElement("test");
        ft.addChildren(element);

        // no nodes are added if empty
        assertEquals(0, element.nodeCount());
    }
}
