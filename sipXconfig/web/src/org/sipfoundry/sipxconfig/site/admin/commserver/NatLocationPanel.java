/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Message;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.BooleanPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class NatLocationPanel extends BaseComponent {
    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Parameter
    public abstract IValidationDelegate getValidator();

    @Parameter
    public abstract ICallback getCallback();

    @Parameter(required = true)
    public abstract Location getLocationBean();

    @Persist("client")
    public abstract boolean isAdvanced();

    public abstract void setBlock(String block);

    @Message("message.invalidStunAddress")
    public abstract String getInvalidStunAddressMsg();

    @Message("message.invalidPublicAddress")
    public abstract String getInvalidPublicAddressMsg();

    @Message("message.invalidRtpRange")
    public abstract String getInvalidRtpRangeMsg();

    @Message("address.type.stun")
    public abstract String getStunLabel();

    @Message("address.type.public")
    public abstract String getPublicAddressLabel();

    public IPropertySelectionModel getAddressTypeModel() {
        return new BooleanPropertySelectionModel(getPublicAddressLabel(), getStunLabel());
    }

    private String calculateBlock() {
        return getLocationBean().isUseStun() ? "stun" : "public";
    }

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        if (!TapestryUtils.isRewinding(cycle, this)) {
            setBlock(calculateBlock());
        }
    }

    public void activate() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        Location location = getLocationBean();
        getLocationsManager().saveLocation(location);
    }
}
