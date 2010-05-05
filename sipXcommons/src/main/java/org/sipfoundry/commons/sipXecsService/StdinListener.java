/**
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.sipXecsService;

import java.util.EventListener;

/**
 * StdinListener: Class defining the interface for a service to receive messages from the supervisor
 *
 * @author Carolyn Beeton
 */

public interface StdinListener extends EventListener {

   /// Process input that has been received
   //  (StdinListener interface)
   public void gotInput(String stdinMsg);
}
