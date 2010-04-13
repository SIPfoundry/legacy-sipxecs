/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.search;

import java.io.File;
import java.io.IOException;

import org.apache.lucene.store.Directory;
import org.apache.lucene.store.RAMDirectory;

/**
 * Memory only indexing - suitable for testing TempIndexSource
 */
public class TempIndexSource extends SimpleIndexSource {
    private Directory m_ramDirectory;

    protected Directory createDirectory(File file_, boolean createDirectory) throws IOException {
        if (m_ramDirectory == null || createDirectory) {
            m_ramDirectory = new RAMDirectory();
        }
        return m_ramDirectory;
    }
}
