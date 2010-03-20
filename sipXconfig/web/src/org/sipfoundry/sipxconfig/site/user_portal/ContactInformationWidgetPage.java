/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.sipfoundry.sipxconfig.components.SipxBasePage;

public abstract class ContactInformationWidgetPage extends SipxBasePage {
    public static final String PAGE = "user_portal/ContactInformationWidgetPage";

    @Asset("/gwt/org.sipfoundry.sipxconfig.userportal.contact_information/nocache.js")
    public abstract IAsset getContactInformationJs();

}
