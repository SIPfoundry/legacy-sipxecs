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

import java.io.File;
import java.io.Reader;

public interface BulkManager {
    public static final String CONTEXT_BEAN_NAME = "bulkManager";

    void insertFromCsv(Reader reader);

    void insertFromCsv(File file, boolean deleteOnImport);
}
