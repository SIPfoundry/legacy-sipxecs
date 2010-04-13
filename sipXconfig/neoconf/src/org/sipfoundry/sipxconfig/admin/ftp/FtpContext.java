/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.ftp;

public interface FtpContext {
    void openConnection();

    String[] listDirectories(String path);

    String[] listFiles(String path);

    void upload(String... names);

    void changeDirectory(String directory);

    void deleteDirectory(String directory);

    void download(String locationPath, String... names);

    void closeConnection();
}
