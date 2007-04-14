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
    void generate(ProfileContext context, String outputFileName);
    
    void copy(String inputFileName, String outputFileName);

    void generate(ProfileContext context, ProfileFilter filter, String outputFileName);
    
    void remove(String outputFileName);
}
