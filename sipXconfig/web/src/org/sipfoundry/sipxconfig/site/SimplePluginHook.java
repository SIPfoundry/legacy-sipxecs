/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

/**
 * Easy way to register your plugin with sipxecs, just include this in you sipxplugin.beans.xml file
 *
 * &lt;bean id="myHook" class="org.sipfoundry.sipxconfig.site.SimplePluginHook"&gt;
 *   &lt;property name="hookId" value="myHook"/&gt;
 * &lt;/bean&gt;
 */
public class SimplePluginHook implements PluginHook {

    private String m_hookId;

    @Override
    public String getHookId() {
        return m_hookId;
    }

    /**
     * Pick a name that would be globally unique.  Bad: myHook  Good: fluxCapacitor
     * @param hookId
     */
    public void setHookId(String hookId) {
        m_hookId = hookId;
    }
}
