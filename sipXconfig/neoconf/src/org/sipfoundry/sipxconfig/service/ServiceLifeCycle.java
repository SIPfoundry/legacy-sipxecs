/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.service;

/**
 * Services can implement these methods to add custom processing.
 */
public interface ServiceLifeCycle {

    void onInit();

    void onDestroy();

    void onStart();

    void onStop();

    void onRestart();
}
