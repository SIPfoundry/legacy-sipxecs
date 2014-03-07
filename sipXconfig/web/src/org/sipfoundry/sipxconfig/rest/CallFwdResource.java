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

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.callgroup.AbstractRing;
import org.sipfoundry.sipxconfig.callgroup.AbstractRing.Type;
import org.sipfoundry.sipxconfig.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.forwarding.Ring;
import org.sipfoundry.sipxconfig.forwarding.Schedule;
import org.springframework.beans.factory.annotation.Required;

public class CallFwdResource extends UserResource {
    private static final Log LOG = LogFactory.getLog(CallFwdResource.class);

    private ForwardingContext m_forwardingContext;

    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        CallSequence callSequence = m_forwardingContext.getCallSequenceForUser(getUser());
        CallFwdBean bean = new CallFwdBean();

        bean.setRings(toRingBeanList(callSequence.getRings()));
        bean.setWithVM(getUser().hasVoicemailPermission());
        bean.setExpiration(callSequence.getCfwdTime());

        LOG.warn("Returning call fwd:\t" + bean);

        return toRepresentation(bean);
    }

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        CallFwdBean bean = fromRepresentation(entity, CallFwdBean.class);
        LOG.warn("Saving call fwd bean:\t" + bean);
        CallSequence callSequence = m_forwardingContext.getCallSequenceForUser(getUser());
        List<AbstractRing> rings = fromRingBeanList(bean.getRings());

        callSequence.replaceRings(rings);
        callSequence.setCfwdTime(bean.getExpiration());
        callSequence.setWithVoicemail(bean.isWithVM());

        LOG.warn("Saving call fwd:\t" + callSequence);

        m_forwardingContext.saveCallSequence(callSequence);
    }

    private static List<RingBean> toRingBeanList(List<AbstractRing> rings) {
        List<RingBean> ringBeans = new ArrayList<CallFwdResource.RingBean>();

        for (AbstractRing ring : rings) {
            RingBean bean = new RingBean();

            bean.setEnabled(ring.isEnabled());
            bean.setExpiration(ring.getExpiration());
            if (ring instanceof Ring) {
                bean.setNumber(((Ring) ring).getNumber());
                Schedule sch = ((Ring) ring).getSchedule();
                if (sch != null) {
                    bean.setScheduleId(sch.getId());
                }
            }
            bean.setType(ring.getType().getName());
            ringBeans.add(bean);
        }

        return ringBeans;
    }

    private List<AbstractRing> fromRingBeanList(List<RingBean> ringBeans) {
        List<AbstractRing> rings = new ArrayList<AbstractRing>();

        for (RingBean bean : ringBeans) {
            Ring ring = new Ring();

            ring.setEnabled(bean.isEnabled());
            ring.setExpiration(bean.getExpiration());
            ring.setNumber(bean.getNumber());
            ring.setType(Type.getEnum(bean.getType()));
            if (bean.getScheduleId() != null) {
                Schedule sch = m_forwardingContext.getScheduleById(bean.getScheduleId());
                if (sch != null) {
                    ring.setSchedule(sch);
                } else {
                    LOG.warn(String.format("Could not find an available schedule with id %d for user %s",
                        bean.getScheduleId(), getUser().getName()));
                }
            }
            rings.add(ring);
        }

        return rings;
    }

    @Required
    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    private static class CallFwdBean {
        private List<RingBean> m_rings;
        private boolean m_withVM;
        private int m_expiration;

        public List<RingBean> getRings() {
            return m_rings;
        }

        public void setRings(List<RingBean> rings) {
            m_rings = rings;
        }

        public boolean isWithVM() {
            return m_withVM;
        }

        public void setWithVM(boolean withVM) {
            m_withVM = withVM;
        }

        public int getExpiration() {
            return m_expiration;
        }

        public void setExpiration(int expiration) {
            m_expiration = expiration;
        }

        @Override
        public String toString() {
            return "CallFwdBean [m_rings=" + m_rings + ", m_withVM=" + m_withVM + ", m_expiration=" + m_expiration
                + "]";
        }
    }

    private static class RingBean {
        private int m_expiration;
        private String m_type;
        private boolean m_enabled;
        private String m_number;
        private Integer m_scheduleId;

        public int getExpiration() {
            return m_expiration;
        }

        public void setExpiration(int expiration) {
            m_expiration = expiration;
        }

        public String getType() {
            return m_type;
        }

        public void setType(String type) {
            m_type = type;
        }

        public boolean isEnabled() {
            return m_enabled;
        }

        public void setEnabled(boolean enabled) {
            m_enabled = enabled;
        }

        public String getNumber() {
            return m_number;
        }

        public void setNumber(String number) {
            m_number = number;
        }

        public Integer getScheduleId() {
            return m_scheduleId;
        }

        public void setScheduleId(Integer scheduleId) {
            m_scheduleId = scheduleId;
        }

        @Override
        public String toString() {
            return "RingBean [m_expiration=" + m_expiration + ", m_type=" + m_type + ", m_enabled=" + m_enabled
                + ", m_number=" + m_number + ", m_scheduleId=" + m_scheduleId + "] ";
        }
    }
}
