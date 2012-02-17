/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.io.IOException;

/**
 * If your ConfigProvider implementation also implements this interface, you can be
 * called after providers have been called and cfengine script have been deployed
 */
public interface PostConfigListener {

    public void postReplicate(ConfigManager manager, ConfigRequest request) throws IOException;

}
