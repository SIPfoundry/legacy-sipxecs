/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.common;

import java.io.File;

/**
 * Cache release info so if can be shown in footer of html page
 */
public class CachedReleaseInfo extends ReleaseInfo {
    private ReleaseInfo m_delegate;
    private long m_date;
    private String m_releaseUpdate;
    private String m_packageInfo;

    public void setDelegate(ReleaseInfo delegate) {
        m_delegate = delegate;
    }

    public String getReleaseUpdate() {
        checkUpdate();
        return m_releaseUpdate;
    }

    public String getPackageInfo() {
        checkUpdate();
        return m_packageInfo;
    }

    void checkUpdate() {
        if (m_date == 0 || new File(m_delegate.getReleaseInfoFile().getAbsolutePath()).lastModified() != m_date) {
            m_releaseUpdate = m_delegate.getReleaseUpdate();
            m_packageInfo = m_delegate.getPackageInfo();
            m_date = m_delegate.getReleaseInfoFile().lastModified();
        }
    }
}
