/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.Reader;
import java.util.List;

import org.apache.commons.collections.Closure;

public interface CsvParser {
    /**
     * Parses CSV file and returns a collection of rows.
     * 
     * Each row is and array of Strings
     * 
     * @param csv reader for CSV data
     * @return list of rows, each row is and array of strings, each String represents a single
     *         field
     */
    List parse(Reader csv);

    /**
     * A preferred version of parsing for potentially larger files Does not build in-memory array.
     * 
     * @param csv reader for CSV data
     * @param closure execute function is called and passed array of Strings for each row
     */
    void parse(Reader csv, Closure closure);
}
