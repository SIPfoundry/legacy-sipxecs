/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
