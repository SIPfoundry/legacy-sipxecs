/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.web.plugin;

import org.sipfoundry.sipxconfig.site.PluginHook;

/**
 * Tapestry 4 page, for dynamic code behind your menu links, should you need any.
 */
public class IMFeatureHook implements PluginHook {

    @Override
    public String getHookId() {
        return "IMFeature";
    }
}