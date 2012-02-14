/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.io.IOException;
import java.io.InputStream;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.io.output.NullOutputStream;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class AgentResults {
    private static final Log LOG = LogFactory.getLog(AgentResults.class);

    public void parse(final InputStream in) {
        Thread t = new Thread(new Runnable() {

            @Override
            public void run() {
                try {
                    IOUtils.copy(in, new NullOutputStream());
                } catch (IOException e) {
                    LOG.fatal("Could not read agent results", e);
                }
            }
        });
        t.start();
    }

    public List<String> getResults() {
        return Collections.emptyList();
    }
}
