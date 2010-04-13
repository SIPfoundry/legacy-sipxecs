/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr.decorators;

import java.util.Locale;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Messages;
import org.sipfoundry.sipxconfig.cdr.Cdr;

public class CdrCallLongDistanceDecorator extends CdrDecorator implements Comparable<CdrCallLongDistanceDecorator> {
    public CdrCallLongDistanceDecorator(Cdr cdr, Locale locale, Messages messages) {
        super(cdr, locale, messages);
    }

    public int compareTo(CdrCallLongDistanceDecorator obj) {
        if (obj == null) {
            return -1;
        }
        String s1 = StringUtils.defaultString(getCallTypeName());
        String s2 = StringUtils.defaultString(obj.getCallTypeName());

        return s1.compareTo(s2);
    }
}
