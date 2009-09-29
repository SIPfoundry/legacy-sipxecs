/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.site.vm.MailboxOperation.MalformedMailboxUrlException;
import org.sipfoundry.sipxconfig.site.vm.MailboxOperation.MoveVoiceMail;
import org.sipfoundry.sipxconfig.site.vm.MailboxOperation.PlayVoiceMail;

public class MailboxOperationTest extends TestCase {

    public void testCreateMailboxOperationFromServletPath() {
        MailboxOperation op = MailboxOperation.createMailboxOperationFromServletPath("/userId/folderId");
        assertEquals("userId", op.getUserId());
        assertEquals("folderId", op.getFolderId());
    }

    public void testCreatePlayOperationFromServletPath() {
        MailboxOperation op = MailboxOperation.createMailboxOperationFromServletPath("/userId/folderId/foo");
        assertEquals(PlayVoiceMail.class, op.getClass());
    }

    public void testCreateDeleteOperationFromServletPath() {
        MailboxOperation op = MailboxOperation.createMailboxOperationFromServletPath("/userId/folderId/foo/delete");
        assertEquals(MoveVoiceMail.class, op.getClass());
    }

    public void testCreateBadOperationFromServletPath() {
        try {
            MailboxOperation.createMailboxOperationFromServletPath("/userId/folderId/foo/unknown");
            fail("expected no operation");
        } catch (MalformedMailboxUrlException expected) {
            assertTrue(true);
        }
    }
}
