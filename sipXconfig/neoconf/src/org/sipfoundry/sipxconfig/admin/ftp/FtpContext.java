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
    public void openConnection();
    public void makeDirectory(String directoryName);
    public String [] list(String path);
    public void upload(String... names);
    public void changeDirectory(String directory);
    public void delete(String... names);
    public void download(String locationPath, String... names);
    public void closeConnection();
    public String getHost();
    public void setHost(String host);
    public String getPassword();
    public void setPassword(String password);
    public String getUserId();
    public void setUserId(String userId);

}
