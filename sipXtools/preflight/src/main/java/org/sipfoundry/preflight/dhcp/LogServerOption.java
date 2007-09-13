/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight.dhcp;

import static org.sipfoundry.preflight.dhcp.DHCPOption.Code.*;

/**
 * [Enter descriptive text here]
 * <p>
 * 
 * @author Mardy Marshall
 */
public class LogServerOption extends ServerOption {
    public LogServerOption() {
        super(LOG_SERVER);
    }

}
