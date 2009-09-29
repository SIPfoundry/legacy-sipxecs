/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import junit.framework.Assert;
import junit.framework.JUnit4TestAdapter;

import org.junit.Test;

public class ServiceStatusMessageHolderTest {

    public static junit.framework.Test suite() {
        return new JUnit4TestAdapter(ServiceStatusMessageHolderTest.class);
    }

    @Test
    public void testAddMessages() {
        List<String> statusStrings = Arrays.asList("version.mismatch: test", "resource.missing:test2");
        List<String> stdOutStrings = Arrays.asList("stdout.msg-1: test3", "stdout.msg-2: test4");
        List<String> stdErrStrings = Arrays.asList("stderr.msg-1: test5");
        List<String> unknownPrefixStrings = Arrays.asList("stdout-msg-3: test6", "stdout-msg.4: test7");

        List<String> allStrings = new ArrayList<String>();
        allStrings.addAll(statusStrings);
        allStrings.addAll(stdOutStrings);
        allStrings.addAll(stdErrStrings);
        allStrings.addAll(unknownPrefixStrings);

        ServiceStatusMessageHolder holder = new ServiceStatusMessageHolder();
        for (String message : allStrings) {
            holder.addMessage(message);
        }

        List<ServiceStatusMessage> stdErrMessages = holder.getStdErr();
        Assert.assertEquals(stdErrStrings.size(), stdErrMessages.size());

        List<ServiceStatusMessage> stdOutMessages = holder.getStdOut();
        Assert.assertEquals(stdOutStrings.size(), stdOutMessages.size());

        List<ServiceStatusMessage> messages = holder.getStatusMessages();
        Assert.assertEquals(statusStrings.size() + unknownPrefixStrings.size(), messages.size());

        Assert.assertEquals(allStrings.size(), holder.getAllMessages().size());
    }
}
