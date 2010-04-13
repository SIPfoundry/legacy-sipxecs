/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import junit.framework.TestCase;

public class AttendantMenuActionTest extends TestCase {

    public void testVxmlParameter() {
        assertEquals("operatoraddr", AttendantMenuAction.OPERATOR.vxmlParameter(null, "somedomain"));
        assertEquals("operatoraddr", AttendantMenuAction.OPERATOR.vxmlParameter("anything", "somedomain"));
        assertEquals("none", AttendantMenuAction.TRANSFER_OUT.vxmlParameter(null, "somedomain"));
        assertEquals("none", AttendantMenuAction.TRANSFER_OUT.vxmlParameter("   ", "somedomain"));
        assertEquals("sip:voltar@somedomain", AttendantMenuAction.TRANSFER_OUT.vxmlParameter("voltar", "somedomain"));
        assertEquals("sip:voltar@somewhere", AttendantMenuAction.TRANSFER_OUT.vxmlParameter("voltar@somewhere", "ignoreddomain"));
    }

}
