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

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.form.validator.BaseValidator;
import org.apache.tapestry.valid.ValidationConstraint;
import org.apache.tapestry.valid.ValidatorException;

/**
 * This validator makes sure that the domain name (be it a FQDN or an IP) is not added as an alias
 * Localized messages are present in sipXconfig-web.properties
 */
public class DomainAliasValidator extends BaseValidator {

    public void validate(IFormComponent field, ValidationMessages messages, Object object) throws ValidatorException {
        String alias = (String) object;
        if (alias == null) {
            return;
        }
        IPage page = field.getPage();
        String domainName = (String) page.getComponent("name").getBinding("value").getObject();
        if (StringUtils.equals(alias, domainName)) {
            throw new ValidatorException(page.getMessages().getMessage("validationMessage.domainAliasValidator"),
                    ValidationConstraint.CONSISTENCY);
        }
    }
}
