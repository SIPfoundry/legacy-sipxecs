/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.callcontroller;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;

public abstract class AbstractMessage {
    public abstract ClientTransaction createAndSend(DialogContext dialogContext,
            String requestType, Operator operator, String body);
}
