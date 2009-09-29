/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.util.io.SqueezeAdaptor;
import org.sipfoundry.sipxconfig.common.DataObjectSource;

/**
 * Hivemind appears to require all squeezers to implement an interface,
 * so here it is
 */
public interface IDaoSqueezeAdapter extends SqueezeAdaptor {

    public void setDataObjectSource(DataObjectSource dao);

}
