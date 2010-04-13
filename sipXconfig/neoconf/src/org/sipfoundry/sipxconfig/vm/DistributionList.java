/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

public class DistributionList {
    // 0-9, not # or *
    private static final int MAX_SIZE = 10;
    private String[] m_extensions;

    public static DistributionList[] createBlankList() {
        DistributionList[] lists = new DistributionList[MAX_SIZE];
        for (int i = 0; i < lists.length; i++) {
            lists[i] = new DistributionList();
        }
        return lists;
    }

    public static Collection<String> getUniqueExtensions(DistributionList... dls) {
        Set<String> uniqueExtensions = new HashSet<String>();
        for (DistributionList distributionList : dls) {
            // distributionList should not be null
            String[] extensions = distributionList.getExtensions();
            if (extensions != null) {
                uniqueExtensions.addAll(Arrays.asList(extensions));
            }
        }
        return uniqueExtensions;
    }

    public String[] getExtensions() {
        return m_extensions;
    }

    public void setExtensions(String[] extensions) {
        m_extensions = extensions;
    }
}
