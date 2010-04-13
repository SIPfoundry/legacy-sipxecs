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
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;

public class LocalizedExtraOptionModelDecorator extends ExtraOptionModelDecorator {
    private static final String LABEL = "label.";

    public LocalizedExtraOptionModelDecorator(Messages messages, String[] options) {
        IPropertySelectionModel model = new StringPropertySelectionModel(options);
        LocalizedOptionModelDecorator localizedModel = new LocalizedOptionModelDecorator(model, messages, LABEL);
        setModel(localizedModel);
        localizedModel.setMessages(messages);
        localizedModel.setResourcePrefix(LABEL);
    }
}
