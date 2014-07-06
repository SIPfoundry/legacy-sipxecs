/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.park;

import java.util.List;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;

public abstract class EditParkOrbit extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "park/EditParkOrbit";

    public abstract ParkOrbitContext getParkOrbitContext();

    public abstract Integer getParkOrbitId();

    public abstract void setParkOrbitId(Integer id);

    public abstract ParkOrbit getParkOrbit();

    public abstract void setParkOrbit(ParkOrbit parkOrbit);

    public abstract List<Location> getParkServers();

    public abstract void setParkServers(List<Location> servers);

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    public void pageBeginRender(PageEvent event_) {
        if (getParkServers() == null) {
            setParkServers(getFeatureManager().getLocationsForEnabledFeature(ParkOrbitContext.FEATURE));
        }
        ParkOrbit orbit = getParkOrbit();
        if (null != orbit) {
            return;
        }
        Integer id = getParkOrbitId();
        if (null != id) {
            ParkOrbitContext context = getParkOrbitContext();
            orbit = context.loadParkOrbit(id);
        } else {
            orbit = getParkOrbitContext().newParkOrbit();
        }
        setParkOrbit(orbit);

        // If no callback was set before navigating to this page, then by
        // default, go back to the ListParkOrbits page
        if (getCallback() == null) {
            setReturnPage(ListParkOrbits.PAGE);
        }
    }

    public void commit() {
        if (!isValid()) {
            return;
        }

        saveValid();
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        if (getParkOrbit().getLocation() == null) {
            delegate.record(new ValidatorException(getMessages().getMessage("message.invalidLocation")));
        }
        return !delegate.getHasErrors();
    }

    private void saveValid() {
        ParkOrbitContext context = getParkOrbitContext();
        ParkOrbit orbit = getParkOrbit();
        context.storeParkOrbit(orbit);
        Integer id = getParkOrbit().getId();
        setParkOrbitId(id);
    }
}
