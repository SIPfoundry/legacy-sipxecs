/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.Collections;

import junit.framework.TestCase;

public class InitTaskListenerTest extends TestCase {

    public void testEventType() {
        InitTaskListenerDummy itl = new InitTaskListenerDummy();
        itl.setTaskNames(Collections.singletonList("bongo"));
        ApplicationInitializedEvent event = new ApplicationInitializedEvent(this);
        itl.onApplicationEvent(event);
        assertFalse(itl.isTriggered());
    }

    public void testEmptyTaskName() {
        InitTaskListenerDummy itl = new InitTaskListenerDummy();
        InitializationTask event = new InitializationTask("bongo");
        try {
            itl.onApplicationEvent(event);
            fail("Expected exception when task list is null.");
        }
        catch (IllegalStateException ise) {
            // expected behavior
        }
    }

    public void testTaskName() {
        InitTaskListenerDummy itl = new InitTaskListenerDummy();
        itl.setTaskNames(Collections.singletonList("kuku"));
        InitializationTask event = new InitializationTask("bongo");
        itl.onApplicationEvent(event);
        assertFalse(itl.isTriggered());
    }

    public void testOnInitTask() {
        InitTaskListenerDummy itl = new InitTaskListenerDummy();
        itl.setTaskNames(Collections.singletonList("bongo"));
        InitializationTask event = new InitializationTask("bongo");
        itl.onApplicationEvent(event);
        assertTrue(itl.isTriggered());
    }

    class InitTaskListenerDummy extends InitTaskListener {
        private boolean triggered;

        public boolean isTriggered() {
            return triggered;
        }

        public void onInitTask(String task) {
            triggered = true;
        }
    }
}
