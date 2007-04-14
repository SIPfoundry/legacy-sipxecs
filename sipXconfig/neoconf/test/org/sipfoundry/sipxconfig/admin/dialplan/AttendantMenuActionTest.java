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
        assertEquals("operatoraddr", AttendantMenuAction.OPERATOR.vxmlParameter(null));
        assertEquals("operatoraddr", AttendantMenuAction.OPERATOR.vxmlParameter("anything"));
        assertEquals("none", AttendantMenuAction.GOTO_EXTENSION.vxmlParameter(null));
        assertEquals("none", AttendantMenuAction.GOTO_EXTENSION.vxmlParameter("   "));
        assertEquals("voltar", AttendantMenuAction.GOTO_EXTENSION.vxmlParameter("voltar"));        
    }

}
