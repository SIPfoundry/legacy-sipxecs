/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk;

import java.io.Reader;

import org.apache.commons.collections.Closure;

public interface BulkParser {
    /**
     * A preferred version of parsing for potentially larger files Does not build in-memory array.
     *
     * @param input reader for data
     * @param closure execute function is called and passed array of Strings for each row
     */
    void parse(Reader input, Closure closure);
}
