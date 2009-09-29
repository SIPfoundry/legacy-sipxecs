/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.asset.AssetFactory;

public interface HivemindContext {
    public AssetFactory getClasspathAssetFactory();

    public void setClasspathAssetFactory(AssetFactory classpathAssetFactory);
}
