/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.phonebook;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Collection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.valid.ValidationConstraint;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager.PhonebookFormat;

public abstract class ExportPhonebookComponent extends BaseComponent {
    private static final Log LOG = LogFactory.getLog(ExportPhonebookComponent.class);

    @InjectObject(value = "service:tapestry.globals.WebResponse")
    public abstract WebResponse getResponse();

    @InjectObject("spring:phonebookManager")
    public abstract PhonebookManager getPhonebookManager();

    @Parameter(required = true)
    public abstract SipxValidationDelegate getValidator();

    /*
     * Required because of the label formatting
     */
    @Parameter(required = true)
    public abstract PhonebookFormat getFormat();

    /**
     * the name of the phonebook file. It can be the username or the name of the phonebook
     */
    @Parameter(required = true)
    public abstract String getName();

    @Parameter(required = true)
    public abstract Collection<PhonebookEntry> getEntries();

    public void export() {
        try {
            String name = String.format("phonebook_%s.%s", getName(), getFormat().getName());
            OutputStream stream = TapestryUtils.getResponseOutputStream(getResponse(), name, "text/x-vcard");
            Collection<PhonebookEntry> entries = getEntries();
            getPhonebookManager().exportPhonebook(entries, stream, getFormat());
            stream.close();
        } catch (IOException e) {
            LOG.error("Cannot export phonebook", e);
            getValidator().record("msg.exportError", ValidationConstraint.CONSISTENCY);
        }
    }

}
