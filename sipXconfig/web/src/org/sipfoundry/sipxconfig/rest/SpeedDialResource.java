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

import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;

public class SpeedDialResource extends UserResource {
    private static final Log LOG = LogFactory.getLog(SpeedDialResource.class);

    private SpeedDialManager m_mgr;
    private PhoneContext m_phoneCtx;
    private ProfileManager m_profileMgr;

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

        if (!dials.isEmpty()) {
            SpeedDial dial = dials.get(0);
            buttons = dial.getButtons();
            groupSpeedDial = false;
        } else {
            groupSpeedDial = true;
            SpeedDial dial = m_mgr.getGroupSpeedDialForUser(getUser(), false);
            if (dial != null) {
                buttons = dial.getButtons();
            } else {
                buttons = Collections.emptyList();
            }
        }

        SpeedDialBean bean = new SpeedDialBean();
        bean.setCanSubscribeToPresence(getUser().hasPermission(PermissionName.SUBSCRIBE_TO_PRESENCE));
        bean.setButtons(buttons);
        bean.setGroupSpeedDial(groupSpeedDial);

        LOG.debug("Returning speed dial:\t" + bean);

        return toRepresentation(bean);
    }

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        SpeedDialBean bean = fromRepresentation(entity, SpeedDialBean.class);
        // do not update if this was not explicitly requested
        boolean updatePhones = bean.isUpdatePhones() != null ? bean.isUpdatePhones() : false;
        LOG.debug("Saving speed dial:\t" + bean);

        if (bean.isGroupSpeedDial()) {
            m_mgr.speedDialSynchToGroup(getUser());
        } else {
            List<SpeedDial> dials = m_mgr.findSpeedDialForUserId(getUser().getId());
            boolean canSubscribe = getUser().hasPermission(PermissionName.SUBSCRIBE_TO_PRESENCE);
            if (!dials.isEmpty()) {
                SpeedDial dial = dials.get(0);
                if (!canSubscribe) {
                    List<Button> existing = dial.getButtons();
                    for (Button b : bean.getButtons()) {
                        if (existing.contains(b)) {
                            for (Button be : existing) {
                                if (be.equals(b)) {
                                    b.setBlf(be.isBlf());
                                    break;
                                }
                            }
                        } else {
                            b.setBlf(false);
                        }
                    }
                }
                dial.setButtons(bean.getButtons());

                m_mgr.saveSpeedDial(dial);
            } else {
                SpeedDial dial = new SpeedDial();
                if (!canSubscribe) {
                    for (Button b : bean.getButtons()) {
                        b.setBlf(false);
                    }
                }
                dial.setUser(getUser());
                dial.setButtons(bean.getButtons());

                m_mgr.saveSpeedDial(dial);
            }
        }
        if (updatePhones) {
            Collection<Phone> phones = m_phoneCtx.getPhonesByUserId(getUser().getId());
            LOG.debug("Updating phones: " + phones);
            @SuppressWarnings("unchecked")
            Collection<Integer> ids = DataCollectionUtil.extractPrimaryKeys(phones);
            m_profileMgr.generateProfiles(ids, true, null);
        }
    }

    public void setMgr(SpeedDialManager mgr) {
        m_mgr = mgr;
    }

    public void setPhoneCtx(PhoneContext phoneCtx) {
        m_phoneCtx = phoneCtx;
    }

    public void setProfileMgr(ProfileManager profileMgr) {
        m_profileMgr = profileMgr;
    }

    // the JSON representation of this is sent to/from the client
    private static class SpeedDialBean {
        private List<Button> m_buttons;
        private boolean m_groupSpeedDial;
        // this is a request parameter
        private Boolean m_updatePhones;
        private Boolean m_canSubscribeToPresence;

        public List<Button> getButtons() {
            return m_buttons;
        }

        public void setButtons(List<Button> buttons) {
            // never set to null, makes further handling harder
            if (buttons != null) {
                m_buttons = buttons;
            } else {
                m_buttons = Collections.emptyList();
            }
        }

        public boolean isGroupSpeedDial() {
            return m_groupSpeedDial;
        }

        public void setGroupSpeedDial(boolean groupSpeedDial) {
            m_groupSpeedDial = groupSpeedDial;
        }

        public Boolean isUpdatePhones() {
            return m_updatePhones;
        }

        @SuppressWarnings("unused")
        public void setUpdatePhones(Boolean updatePhones) {
            m_updatePhones = updatePhones;
        }

        public Boolean isCanSubscribeToPresence() {
            return m_canSubscribeToPresence;
        }

        public void setCanSubscribeToPresence(Boolean canSubscribeToPresence) {
            m_canSubscribeToPresence = canSubscribeToPresence;
        }

        @Override
        public String toString() {
            return "SpeedDialBean [m_buttons=" + m_buttons + ", m_groupSpeedDial=" + m_groupSpeedDial
                + ", m_updatePhones=" + m_updatePhones + ", m_canSubscribeToPresence=" + m_canSubscribeToPresence
                + "]";
        }
    }
}
