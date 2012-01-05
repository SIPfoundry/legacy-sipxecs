/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.proxy;

import org.sipfoundry.sipxconfig.site.PluginHook;

public class EditProxyHook implements PluginHook {

    @Override
    public String getHookId() {
        return "EditProxyMenu";
    }
}
