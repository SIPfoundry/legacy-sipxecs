/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

/**
 * Adds navigation pages to menu.  Today, just admin, but hope to add user menu at some point.
 */
public interface NavigationProvider {

    /**
     * Page name of where all your menu render blocks are stored. See ExamplePluginMenu.page
     * Example: "EditWidgetPage".  Do not include prefix "/plugin" or suffix ".page"
     */
    public String getAdminMenuPageId();

}
