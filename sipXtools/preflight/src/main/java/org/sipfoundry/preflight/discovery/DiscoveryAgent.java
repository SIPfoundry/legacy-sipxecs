/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight.discovery;


import javax.sip.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public interface DiscoveryAgent {
    public void discover();
    
    public void processResponse(ResponseEvent responseEvent);

    public void processTimeout(TimeoutEvent timeoutEvent);

    public void processTransactionTerminated(TransactionTerminatedEvent  transactionTerminatedEvent);
    
    public void terminate();
    
}
