/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.lang.StringUtils;

/**
 * Version information is kept in jar manifest file which is created during build process. See
 * manifest task in neoconf/build.xml for more details.
 */
public class VersionInfo {


    /**
     * @return Major and minor version number
     */
    public String getVersion() {
        return StringUtils.defaultString(getClass().getPackage().getSpecificationVersion());
    }

    /**
     * Split version into convienient ids for comparison
     *
     * @return versions ids as string array
     */
    public Integer[] getVersionIds() {
        return versionStringToVersionIds(getVersion());
    }

    static Integer[] versionStringToVersionIds(String version) {
        if (StringUtils.isBlank(version)) {
            return new Integer[0];
        }
        String[] sids = version.split("\\.");
        Integer[] ids = new Integer[sids.length];
        for (int i = 0; i < ids.length; i++) {
            ids[i] = new Integer(sids[i]);
        }
        return ids;
    }

    String getBuildStamp() {
        return getClass().getPackage().getImplementationVersion();
    }

    /**
     * @return more specific build information
     */
    public String[] getVersionDetails() {
        String[] implVer = StringUtils.defaultString(getBuildStamp()).split(" ");
        String[] details = new String[implVer.length + 1];
        System.arraycopy(implVer, 0, details, 1, implVer.length);
        details[0] = getVersion();
        return details;
    }
}
