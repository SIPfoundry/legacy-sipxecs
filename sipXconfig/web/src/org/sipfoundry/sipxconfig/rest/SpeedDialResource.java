/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.rest;

import static org.sipfoundry.sipxconfig.rest.JacksonConvert.fromRepresentation;
import static org.sipfoundry.sipxconfig.rest.JacksonConvert.toRepresentation;

import java.util.Collections;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;

public class SpeedDialResource extends UserResource {
    private static final Log LOG = LogFactory.getLog(ImSettingsResource.class);

    private SpeedDialManager m_mgr;

    @Override
    public boolean allowPost() {
        return false;
    }

    @Override
    public boolean allowDelete() {
        return false;
    }

    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        List<SpeedDial> dials = m_mgr.findSpeedDialForUserId(getUser().getId());
        List<Button> buttons;
        boolean groupSpeedDial;
        Integer id;

        if (!dials.isEmpty()) {
            SpeedDial dial = dials.get(0);
            buttons = dial.getButtons();
            id = dial.getId();
            groupSpeedDial = false;
        } else {
            groupSpeedDial = true;
            SpeedDial dial = m_mgr.getGroupSpeedDialForUser(getUser(), false);
            if (dial != null) {
                buttons = dial.getButtons();
                id = dial.getId();
            } else {
                buttons = Collections.emptyList();
                id = null;
            }
        }

        SpeedDialBean bean = new SpeedDialBean();
        bean.setId(id);
        bean.setButtons(buttons);
        bean.setGroupSpeedDial(groupSpeedDial);

        LOG.debug("Returning speed dial:\t" + bean);

        return toRepresentation(bean);
    }

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        SpeedDialBean bean = fromRepresentation(entity, SpeedDialBean.class);

        LOG.debug("Saving speed dial:\t" + bean);

        List<SpeedDial> dials = m_mgr.findSpeedDialForUserId(getUser().getId());
        if (!dials.isEmpty()) {
            SpeedDial dial = dials.get(0);
            dial.setUniqueId(bean.getId());
            dial.setButtons(bean.getButtons());
            dial.setUser(getUser());

            if (bean.isGroupSpeedDial()) {
                m_mgr.speedDialSynchToGroup(dial);
            } else {
                m_mgr.saveSpeedDial(dial);
            }
        } else {
            // trying to save, but no user speed dial found
            throw new ResourceException(Status.CLIENT_ERROR_NOT_FOUND);
        }
    }

    public void setMgr(SpeedDialManager mgr) {
        m_mgr = mgr;
    }

    // the JSON representation of this is sent to/from the client
    private static class SpeedDialBean {
        private Integer m_id;
        private List<Button> m_buttons;
        private boolean m_groupSpeedDial;

        public Integer getId() {
            return m_id;
        }

        public void setId(Integer id) {
            m_id = id;
        }

        public List<Button> getButtons() {
            return m_buttons;
        }

        public void setButtons(List<Button> buttons) {
            // never set to null, makes further handling harder
            if (buttons != null) {
                this.m_buttons = buttons;
            } else {
                this.m_buttons = Collections.emptyList();
            }
        }

        public boolean isGroupSpeedDial() {
            return m_groupSpeedDial;
        }

        public void setGroupSpeedDial(boolean groupSpeedDial) {
            this.m_groupSpeedDial = groupSpeedDial;
        }

        @Override
        public String toString() {
            return "SpeedDialBean [m_id=" + m_id + ", m_buttons=" + m_buttons + ", m_groupSpeedDial="
                + m_groupSpeedDial + "]";
        }
    }
}
