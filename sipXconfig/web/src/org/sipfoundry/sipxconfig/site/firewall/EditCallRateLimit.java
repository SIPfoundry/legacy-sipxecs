/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.firewall;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.NewEnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.firewall.CallRateLimit;

public abstract class EditCallRateLimit extends BaseComponent {

    public abstract IPropertySelectionModel getSipMethodModel();

    public abstract void setSipMethodModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getIntervalModel();

    public abstract void setIntervalModel(IPropertySelectionModel model);

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        if (getSipMethodModel() == null) {
            NewEnumPropertySelectionModel model = new NewEnumPropertySelectionModel();
            model.setEnumType(CallRateLimit.SipMethod.class);
            setSipMethodModel(model);
        }

        if (getIntervalModel() == null) {
            NewEnumPropertySelectionModel intervalModel = new NewEnumPropertySelectionModel();
            intervalModel.setEnumType(CallRateLimit.CallRateInterval.class);
            setIntervalModel(intervalModel);
        }
    }
}
