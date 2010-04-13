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

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Messages;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.valid.IValidator;
import org.apache.tapestry.valid.ValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.UserException;

/**
 * SipXconfig version of the validator. It does not decorate labels. SipxValidationDelegate
 */
public class SipxValidationDelegate extends ValidationDelegate {
    /**
     * This value is defined in sipXconfig/web/context/css/sipxconfig.css
     */
    private static final String ERROR_CLASS = "user-error";

    private final String m_suffix;
    private final boolean m_decorateLabels;
    private String m_success;

    public SipxValidationDelegate() {
        this("*", false);
    }

    public SipxValidationDelegate(String suffix, boolean decorateLabels) {
        m_decorateLabels = decorateLabels;
        m_suffix = suffix;
    }

    @Override
    public void writeLabelPrefix(IFormComponent component, IMarkupWriter writer, IRequestCycle cycle) {
        if (m_decorateLabels) {
            super.writeLabelPrefix(component, writer, cycle);
        }
    }

    @Override
    public void writeLabelSuffix(IFormComponent component, IMarkupWriter writer, IRequestCycle cycle) {
        if (m_decorateLabels) {
            super.writeLabelSuffix(component, writer, cycle);
        }
    }

    @Override
    public void writeSuffix(IMarkupWriter writer, IRequestCycle cycle_, IFormComponent component_,
            IValidator validator_) {
        if (isInError()) {
            writer.printRaw("&nbsp;");
            writer.begin("span");
            writer.attribute("class", ERROR_CLASS);
            writer.print(m_suffix);
            writer.end();
        }
    }

    @Override
    public void clear() {
        super.clear();
        m_success = null;
    }

    @Override
    public void clearErrors() {
        super.clearErrors();
        m_success = null;
    }

    public void recordSuccess(String success) {
        m_success = success;
    }

    public String getSuccess() {
        return m_success;
    }

    public boolean getHasSuccess() {
        return !getHasErrors() && StringUtils.isNotBlank(m_success);
    }

    public void record(UserException e, Messages messages) {
        String msg = getFormattedMsg(e, messages);
        ValidatorException ve = new ValidatorException(msg);
        record(ve);
    }

    /**
     * Prepares message to be recorded as validation exception
     *
     * It will also check for localization for parameters. If a localization will be found for a
     * parameter it will be translated otherwise not.
     *
     * @param e user exception
     * @param messages optional reference to message store
     * @return user visible message
     */
    private String getFormattedMsg(UserException e, Messages messages) {
        if (messages == null) {
            // no localized resources available
            return e.getMessage();
        }
        return LocalizationUtils.localizeException(messages, e);
    }
}
