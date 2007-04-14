/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.translator.Translator;
import org.apache.tapestry.form.validator.Validator;
import org.sipfoundry.sipxconfig.components.SerialNumberTranslator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class GatewayForm extends BaseComponent {
    @Parameter(required = true)
    public abstract Gateway getGateway();

    @InjectObject(value = "spring:gatewayContext")
    public abstract GatewayContext getGatewayContext();

    public List<Validator> getSerialNumberValidators() {
        return TapestryUtils.getSerialNumberValidators(getGateway().getModel());
    }

    public Translator getSerialNumberTranslator() {
        return new SerialNumberTranslator(getGateway().getModel());
    }
}
