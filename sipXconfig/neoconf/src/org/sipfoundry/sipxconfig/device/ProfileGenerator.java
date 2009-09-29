/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

public interface ProfileGenerator {
    void copy(ProfileLocation location, String inputFileName, String outputFileName);

    void copy(ProfileLocation location, String inputDirPath, String inputFileName, String outputFileName);

    void generate(ProfileLocation location, ProfileContext context, ProfileFilter filter,
            String outputFileName);
}
