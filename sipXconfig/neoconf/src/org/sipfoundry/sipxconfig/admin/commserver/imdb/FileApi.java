/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

public interface FileApi {
    /**
     * Replaces/Create a specified file (path and filename) on the system.
     *
     * @param hostname FQDN of the calling host to be checked as an SSL trusted peer and against
     *        an explicit list of hosts allowed to make requests.
     * @param filename full path and filename of the file to add or replace.
     * @param filePermissions permissions to place on the file for user, group and other. This is
     *        typically specified as an Octet but we are restricted to the XML-RPC types. The
     *        Octet value must be converted to an integer value and passed in.
     * @param content The actual contents of the file in base 64 encoding.
     *
     * @return true if the specified file was successfully replaced or created. Returns false
     *         otherwise.
     */
    boolean replace(String hostname, String filename, int filePermissions, String content);
}
