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
}
