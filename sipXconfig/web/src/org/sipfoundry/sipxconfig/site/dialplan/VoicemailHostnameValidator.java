/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.form.validator.BaseValidator;
import org.apache.tapestry.valid.ValidationConstraint;
import org.apache.tapestry.valid.ValidatorException;

public class VoicemailHostnameValidator extends BaseValidator {

    public void validate(IFormComponent field, ValidationMessages messages, Object object)
        throws ValidatorException {
        String serverAddress = (String) object;
        String mediaServerType = (String) field.getPage().getComponent("mediaServer").getBinding(
                "value").getObject();
        if ("exchangeUmMediaServer".equals(mediaServerType) && StringUtils.isEmpty(serverAddress)) {
            throw new ValidatorException(getMessage(), ValidationConstraint.CONSISTENCY);
        }
    }

    public boolean getAcceptsNull() {
        return true;
    }
}
