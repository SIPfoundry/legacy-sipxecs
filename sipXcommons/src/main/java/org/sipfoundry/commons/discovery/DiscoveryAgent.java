/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.discovery;


import javax.sip.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public interface DiscoveryAgent {
    public void discover();

    public void processPingResponse();

    public void processPingTimeout();

    public void processSIPResponse(ResponseEvent responseEvent);

    public void processSIPTimeout(TimeoutEvent timeoutEvent);

    public void processTransactionTerminated(TransactionTerminatedEvent  transactionTerminatedEvent);

    public void terminate();

}
