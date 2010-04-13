/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.sbc;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbcRegistrationRecord;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbcStatistics;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class BridgeSbcStatisticsPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "sbc/BridgeSbcStatisticsPage";

    @InjectObject(value = "spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject("spring:bridgeSbcStatistics")
    public abstract BridgeSbcStatistics getBridgeSbcStatistics();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setCallCount(int callCount);

    public abstract void setSource(Object object);

    public abstract BridgeSbcRegistrationRecord getRow();

    public abstract int getCurrentLocationId();

    public abstract void setCurrentLocationId(int id);

    public abstract LocationSelectionModel getLocationSelectionModel();

    public abstract void setLocationSelectionModel(LocationSelectionModel locationSelectionModel);

    public void pageBeginRender(PageEvent event_) {
        if (!getSbcDeviceManager().isInternalSbcEnabled()) {
            return;
        }

        List<BridgeSbc> bridgeSbcs = getSbcDeviceManager().getBridgeSbcs();

        if (getCurrentLocationId() == 0) {
            setCurrentLocationId(bridgeSbcs.get(0).getLocation().getId());
        }

        BridgeSbc bridgeSbc = getSbcDeviceManager().getBridgeSbc(getLocationsManager().
                getLocation(getCurrentLocationId()));

        int callCount = 0;
        List<BridgeSbcRegistrationRecord> bridgeSbcRegistrationRecords = null;
        try {
            callCount = getBridgeSbcStatistics().getCallCount(bridgeSbc);
            BridgeSbcRegistrationRecord[] bridgeSbcRegistrationRecordArray = getBridgeSbcStatistics().
                getRegistrationRecords(bridgeSbc);
            if (null != bridgeSbcRegistrationRecordArray) {
                bridgeSbcRegistrationRecords = Arrays.asList(bridgeSbcRegistrationRecordArray);
            }
        } catch (Exception e) {
            UserException exception = new UserException("&error.xml.rpc");
            getValidator().record(exception, getMessages());
        }

        setCallCount(callCount);
        if (null == bridgeSbcRegistrationRecords) {
            bridgeSbcRegistrationRecords = new ArrayList<BridgeSbcRegistrationRecord>();
        }
        setSource(bridgeSbcRegistrationRecords);
        setLocationSelectionModel(new LocationSelectionModel(bridgeSbcs));
    }

    public static class LocationSelectionModel extends ObjectSelectionModel {
        LocationSelectionModel(List<BridgeSbc> bridgeSbcs) {
            List<Location> locations = new ArrayList<Location>();
            for (int i = 0; i < bridgeSbcs.size(); i++) {
                locations.add(bridgeSbcs.get(i).getLocation());
            }

            setCollection(locations);
            setLabelExpression("fqdn");
            setValueExpression("id");
        }
    }
}
