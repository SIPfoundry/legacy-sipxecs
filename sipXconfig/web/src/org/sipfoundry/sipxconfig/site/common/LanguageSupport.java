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

import java.util.Locale;

public interface LanguageSupport {

    /**
     * Resolves the specified locale object to a locale label suitable for display
     */
    String resolveLocaleName(Locale locale);

    /**
     * Resolves the specified locale name to a local label suitable for display.  The
     * locale name should be of the form "xx", or "xx-YY", where xx is the two character
     * language identifier and YY is the two character country identifier.
     */
    String resolveLocaleName(String localeName);
}
