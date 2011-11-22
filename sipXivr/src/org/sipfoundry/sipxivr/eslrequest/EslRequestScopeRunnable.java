/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxivr.eslrequest;


public abstract class EslRequestScopeRunnable implements Runnable {

    public final void run() {
        try {
            runEslRequest();
        } finally {
            EslRequestScopeContextHolder.currentEslRequestScopeAttributes().clear();
        }
    }

    public abstract void runEslRequest();
}
