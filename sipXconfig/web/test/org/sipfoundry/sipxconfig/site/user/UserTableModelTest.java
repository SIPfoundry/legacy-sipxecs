/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;

import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ognl.ExpressionTableColumn;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class UserTableModelTest extends TestCase {

    public void testGetCurrentPageRows() {
        User[] page1Array = new User[] {
            new User(), new User()
        };
        User[] page2Array = new User[] {
            new User(), new User()
        };

        List page1 = Arrays.asList(page1Array);
        List page2 = Arrays.asList(page2Array);

        Integer groupId = new Integer(5);

        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);

        coreContext.loadUsersByPage(null, null, null, 0, 1, "userName", true);
        coreContextCtrl.andReturn(page1);

        coreContext.loadUsersByPage(null, groupId, null, 1, 1, "userName", true);
        coreContextCtrl.andReturn(page2);

        coreContextCtrl.replay();

        UserTableModel model = new UserTableModel();
        model.setCoreContext(coreContext);
        ITableColumn col = new ExpressionTableColumn("userName", "userName", true, null);
        Iterator i1 = model.getCurrentPageRows(0, 1, col, true);
        assertEquals(page1Array[0], i1.next());
        assertEquals(page1Array[1], i1.next());
        assertFalse(i1.hasNext());

        model.setGroupId(groupId);
        Iterator i2 = model.getCurrentPageRows(1, 1, col, true);
        assertEquals(page2Array[0], i2.next());
        assertEquals(page2Array[1], i2.next());
        assertFalse(i2.hasNext());

        coreContextCtrl.verify();
    }
}
