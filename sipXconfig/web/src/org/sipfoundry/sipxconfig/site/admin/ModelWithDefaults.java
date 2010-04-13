/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.hivemind.Messages;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;

public class ModelWithDefaults extends LocalizedOptionModelDecorator {
    public static final String DEFAULT = "default";
    public static final String LABEL = "label.";

    public ModelWithDefaults(Messages messages, String[] options) {
        String[] opts = options;
        if (opts.length == 0) {
            opts = new String[] {
                DEFAULT
            };
        }
        setModel(new StringPropertySelectionModel(opts));
        setMessages(messages);
        setResourcePrefix(LABEL);
    }
}
