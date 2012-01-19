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


import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitConfiguration;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;

public abstract class EditParkOrbit extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "park/EditParkOrbit";

    public abstract ParkOrbitContext getParkOrbitContext();

    public abstract ParkOrbitConfiguration getParkConfig();

    public abstract Integer getParkOrbitId();

    public abstract void setParkOrbitId(Integer id);

    public abstract ParkOrbit getParkOrbit();

    public abstract void setParkOrbit(ParkOrbit parkOrbit);

    public void pageBeginRender(PageEvent event_) {
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
