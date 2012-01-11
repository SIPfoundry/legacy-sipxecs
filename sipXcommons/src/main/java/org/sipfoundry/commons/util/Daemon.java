/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.commons.util;

/**
 * Allow a class that has a main(String[] args) method easily turn itself into a long standing
 * daemon app. This requires some coordination w/startup script to pass pidfile so daemon runner
 * can tell is there is already an instance running.
 * 
 * Inspired by Apache Commons Daemon interface, this is a subset of that. Apache Commons was
 * investigated but version that was available w/CentOS 6 was 1.0.2 which is old and had a bug
 * where pid file wasn't created and not available to java. Building a newer version of the
 * package was difficult because of all the dependencies.
 */
public interface Daemon {
    
    public void start();
    
    public void stop();
}
