/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
    private String m_featureId;

    @Override
    public String getHookId() {
        return m_hookId;
    }

    @Override
    public String getFeatureId() {
        return m_featureId;
    }


    /**
     * Pick a name that would be globally unique.  Bad: myHook  Good: fluxCapacitor
     * @param hookId
     */
    public void setHookId(String hookId) {
        m_hookId = hookId;
    }
}
