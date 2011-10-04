/*
 * Copyright (C) 2011 Your Company Here,
 * Licensed to the User under the AGPL license.
 * $
 *
 */
package org.sipfoundry;

import org.sipfoundry.sipxconfig.common.CoreContext;

/**
 * Java class do perform any function you desire.
 *
 * You can get access to *any* of the several hundred spring beans including beans from other
 * plugins. Simply add an accessor method (getFoo/setFoo pair) to this file and add appropriate
 * spring bean name to sipxplugin.beans.xml. To find the bean names for objects, review all the
 * *.beans.xml files in the source or jar files
 */
public class Example {
    private CoreContext m_coreContext;

    public String hello() {
        return "hello world";
    }

    public int getNumberOfUsers() {
        return m_coreContext.getAllUsersCount();
    }

    public CoreContext getCoreContext() {
        return m_coreContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
