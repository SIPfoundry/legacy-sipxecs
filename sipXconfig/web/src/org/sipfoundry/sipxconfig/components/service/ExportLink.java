/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components.service;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.engine.DirectServiceParameter;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.link.DirectLink;

public abstract class ExportLink extends DirectLink {
    public abstract IEngineService getService();

    public ILink getLink(IRequestCycle cycle) {
        Object[] params = DirectLink.constructServiceParameters(getParameters());
        DirectServiceParameter dsp = new DirectServiceParameter(this, params);
        return getService().getLink(false, dsp);
    }
}
