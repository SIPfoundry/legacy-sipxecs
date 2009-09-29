/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import org.apache.velocity.VelocityContext;

public class DistributionListsWriter extends XmlWriterImpl<DistributionList[]> {

    public DistributionListsWriter() {
        setTemplate("mailbox/distribution.vm");
    }

    @Override
    protected void addContext(VelocityContext context, DistributionList[] lists) {
        context.put("lists", lists);
    }
}
