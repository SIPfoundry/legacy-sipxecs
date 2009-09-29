/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.BeanPropertySelectionModel;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.validator.Validator;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class BridgeSelector extends BaseComponent {

    @Parameter(required = false)
    public abstract IValidationDelegate getValidator();

    @Parameter(required = false)
    public abstract Collection<Validator> getValidators();

    @Parameter(required = true)
    public abstract Bridge getBridge();
    public abstract void setBridge(Bridge bridge);

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public IPropertySelectionModel getConferenceBridgesModel() {
        List<Bridge> bridges = getConferenceBridgeContext().getBridges();
        BeanPropertySelectionModel bridgeModel = new BeanPropertySelectionModel(bridges, "name");
        ExtraOptionModelDecorator model = new ExtraOptionModelDecorator();
        model.setModel(bridgeModel);
        model.setExtraLabel(getMessages().getMessage("option.selectBridge"));
        model.setExtraOption(null);

        if (bridges.size() == 1) {
            setBridge(bridges.get(0));
        }

        return model;
    }
}
