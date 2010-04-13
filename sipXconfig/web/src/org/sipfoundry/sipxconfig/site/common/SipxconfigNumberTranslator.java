/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import java.text.Format;
import java.util.Locale;

import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.translator.NumberTranslator;

public class SipxconfigNumberTranslator extends NumberTranslator {

    public SipxconfigNumberTranslator() {
        super();
    }

    public SipxconfigNumberTranslator(String initializer) {
        super(initializer);
    }

    /**
     * Format the given object as a Number, ignoring the omitZero flag in the NumberTranslator superclass
     */
    @Override
    protected String formatObject(IFormComponent field, Locale locale, Object object) {
        Format format = getFormat(locale);
        return format.format(object);
    }
}
