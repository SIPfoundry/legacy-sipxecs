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

public class HivemindContextImpl implements HivemindContext {
    private AssetFactory m_classpathAssetFactory;

    public AssetFactory getClasspathAssetFactory() {
        return m_classpathAssetFactory;
    }

    public void setClasspathAssetFactory(AssetFactory classpathAssetFactory) {
        m_classpathAssetFactory = classpathAssetFactory;
    }
}
