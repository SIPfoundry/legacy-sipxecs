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

import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.Locale;

import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.valid.ValidatorException;

/**
 * Integer translator using locale acceptable symbols.
 *
 * Undesired feature: Does not complain about decimal values, just drops them.
 */
public class IntTranslator extends SipxconfigNumberTranslator {

    public IntTranslator() {
    }

    public IntTranslator(String initializer) {
        super(initializer);
    }

    @Override
    public DecimalFormat getDecimalFormat(Locale locale) {
        // assumes NumberFormat actually returns a DecimalFormat, which is does
        DecimalFormat format = (DecimalFormat) NumberFormat.getIntegerInstance(locale);
        format.setGroupingUsed(false);
        return format;
    }

    @Override
    protected Object parseText(IFormComponent field, ValidationMessages messages, String text)
        throws ValidatorException {
        // Allow nothing but digits: [0-9], for everything else through exception.
        if (!text.matches("(\\d)+")) {
            throw new ValidatorException(buildMessage(messages, field, getMessageKey()), getConstraint());
        } else {
            return super.parseText(field, messages, text);
        }
    }
}
