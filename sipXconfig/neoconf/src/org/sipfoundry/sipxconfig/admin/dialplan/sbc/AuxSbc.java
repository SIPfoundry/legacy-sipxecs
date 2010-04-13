/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

/**
 * Auxiliary SBC has the same fields as default SBC. The only difference is that in case of
 * default SBC one specifies Inranet routes, that are *not* sent trough the SBC. In case of
 * auxiliary SBC one specifis the routes that *are* sent through SBC.
 */
public class AuxSbc extends Sbc {
}
