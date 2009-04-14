/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.collections.iterators.EmptyIterator;
import org.apache.commons.lang.StringUtils;

public final class SipxCollectionUtils {

    /** Singleton class with static methods, don't allow anyone to instantiate */
    private SipxCollectionUtils() {
    }

    /** Return the Collection size. Return 0 if the Collection is null. */
    public static int safeSize(Collection coll) {
        return coll != null ? coll.size() : 0;
    }

    public static boolean safeIsEmpty(Collection coll) {
        return safeSize(coll) == 0;
    }

    /** Return a Collection iterator. Return an empty iterator if the Collection is null. */
    public static Iterator safeIterator(Collection coll) {
        return coll != null ? coll.iterator() : EmptyIterator.INSTANCE;
    }

    /**
     * Displays as much information about the array as we can without overwhelming log file with
     * useless data
     */
    public static String arrayToShortString(Object[] items, int len) {
        String[] strItems = new String[items.length];
        for (int i = 0; i < items.length; i++) {
            Object item = items[i];
            String itemRepr;
            if (item instanceof Object[]) {
                itemRepr = Arrays.deepToString((Object[]) item);
            } else {
                itemRepr = item.toString();
            }
            strItems[i] = StringUtils.abbreviate(itemRepr, len);
        }
        return "[" + StringUtils.join(strItems, ", ") + "]";
    }
}
