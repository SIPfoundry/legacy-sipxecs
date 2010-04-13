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
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.BooleanPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.nattraversal.NatLocation;

import static org.apache.commons.lang.StringUtils.isEmpty;

public abstract class NatLocationPanel extends BaseComponent implements PageBeginRenderListener {
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

    public abstract NatLocation getNatLocation();

    public abstract void setBlock(String block);

    public abstract void setNatLocation(NatLocation natLocation);

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

    public void pageBeginRender(PageEvent event) {
        NatLocation nat = getNatLocation();
        if (nat == null) {
            nat = getLocationBean().getNat();
            setNatLocation(nat);
        }
    }

    private String calculateBlock() {
        return getNatLocation().isUseStun() ? "stun" : "public";
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

        NatLocation natLocation = getNatLocation();
        if (natLocation.isUseStun()) {
            if (isEmpty(natLocation.getStunAddress())) {
                throw new UserException(getInvalidStunAddressMsg());
            }
        } else {
            if (isEmpty(natLocation.getPublicAddress())) {
                throw new UserException(getInvalidPublicAddressMsg());
            }
        }
        if (natLocation.getStartRtpPort() > natLocation.getStopRtpPort()) {
            throw new UserException(getInvalidRtpRangeMsg());
        }

        Location location = getLocationBean();
        getLocationsManager().storeNatLocation(location, natLocation);
    }
}
