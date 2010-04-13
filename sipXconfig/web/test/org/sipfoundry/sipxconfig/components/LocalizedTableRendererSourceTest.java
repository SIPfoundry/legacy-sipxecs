/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

import org.apache.hivemind.Messages;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumn;
import org.apache.tapestry.valid.RenderString;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class LocalizedTableRendererSourceTest extends TestCase {

    private final class TestColumn extends SimpleTableColumn {
        private TestColumn(String name) {
            super(name);
        }

        public Object getColumnValue(Object objRow) {
            if ("value1".equals(objRow)) {
                return null;
            }
            if ("value2".equals(objRow)) {
                return "bongo";
            }
            fail("should be only called with value1, or value2 as parameter");
            return null;

        }
    }

    public void testGetRenderer() {
        IMocksControl messagesCtrl = EasyMock.createControl();
        Messages messages = messagesCtrl.createMock(Messages.class);

        // PORT
        //messages.getMessage("prefix.bongo", "bongo");
        messages.getMessage("prefix.bongo");

        messagesCtrl.andReturn("kuku");
        messagesCtrl.replay();

        // class mock does not work here...
        ITableColumn column = new TestColumn(null);

        LocalizedTableRendererSource source = new LocalizedTableRendererSource(messages, "prefix");

        RenderString renderer1 = (RenderString) source.getRenderer(null, null, column, "value1");
        assertEquals("&nbsp;", renderer1.getString());

        RenderString renderer2 = (RenderString) source.getRenderer(null, null, column, "value2");
        assertEquals("kuku", renderer2.getString());

        messagesCtrl.verify();
    }

    public void testGetRendererNoPrefix() {
        IMocksControl messagesCtrl = EasyMock.createControl();
        Messages messages = messagesCtrl.createMock(Messages.class);
        messages.getMessage("bongo");
        messagesCtrl.andReturn("kuku");
        messagesCtrl.replay();

        // class mock does not work here...
        ITableColumn column = new TestColumn(null);

        LocalizedTableRendererSource source = new LocalizedTableRendererSource(messages);

        RenderString renderer2 = (RenderString) source.getRenderer(null, null, column, "value2");
        assertEquals("kuku", renderer2.getString());

        messagesCtrl.verify();
    }


}
