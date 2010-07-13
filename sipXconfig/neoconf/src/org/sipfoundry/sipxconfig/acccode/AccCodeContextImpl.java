/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acccode;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.AuthorizationCodeRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxAccCodeService;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

public class AccCodeContextImpl extends SipxHibernateDaoSupport implements AccCodeContext {

    public static final Log LOG = LogFactory.getLog(AccCodeContextImpl.class);

    private static final String ALERT_INFO = "sipXacccode";

    private boolean m_enabled = true;
    private LocationsManager m_locationsManager;

    private SipxServiceManager m_sipxServiceManager;

    private SipxAccCodeService m_acccodeService;

    private DialPlanActivationManager m_dialPlanActivationManager;

    private void onLocationSpecificServiceDelete(LocationSpecificService locationService) {
        SipxService service = locationService.getSipxService();
    }

    private void onLocationDelete(Location location) {
    }

    private void onLocationSave(Location location) {
    }

    public void onDelete(Object entity) {
    }

    public void onSave(Object entity) {
        if (entity instanceof Location) {
            onLocationSave((Location) entity);
        }
    }

    public List< ? extends DialingRule> getDialingRules() {
        List<DialingRule> dialingRules = new ArrayList<DialingRule>();
        String prefix = getAuthCodePrefix();
        if (StringUtils.isEmpty(prefix)) {
            return Collections.emptyList();
        }
        AuthorizationCodeRule rule = new AuthorizationCodeRule(prefix, getSipxFreeswitchAddressAndPort(), "auth");
        rule.appendToGenerationRules(dialingRules);
        return dialingRules;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getAuthCodePrefix() {
        String prefix;
        m_acccodeService = (SipxAccCodeService) m_sipxServiceManager
                 .getServiceByBeanId(SipxAccCodeService.BEAN_ID);
        prefix = m_acccodeService.getSettingValue(SipxAccCodeService.AUTH_CODE_PREFIX);

        return prefix;
    }

    public void setAuthCodePrefix(String prefix) {
        m_acccodeService = (SipxAccCodeService) m_sipxServiceManager
                 .getServiceByBeanId(SipxAccCodeService.BEAN_ID);
        m_acccodeService.setAuthCodePrefix(prefix);
    }

    public String getAuthCodeAliases() {
        String aliases;
        m_acccodeService = (SipxAccCodeService) m_sipxServiceManager
                 .getServiceByBeanId(SipxAccCodeService.BEAN_ID);
        aliases = m_acccodeService.getSettingValue(SipxAccCodeService.AUTH_CODE_ALIASES);

        return aliases;
    }

    public void setAuthCodeAliases(String aliases) {
        m_acccodeService = (SipxAccCodeService) m_sipxServiceManager
                 .getServiceByBeanId(SipxAccCodeService.BEAN_ID);
        m_acccodeService.setAuthCodeAliases(aliases);
    }


    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setDialPlanActivationManager(DialPlanActivationManager dialPlanActivationManager) {
        m_dialPlanActivationManager = dialPlanActivationManager;
    }

    private SipxFreeswitchService getSipxFreeswitchService() {
        return (SipxFreeswitchService) m_sipxServiceManager.getServiceByBeanId(SipxFreeswitchService.BEAN_ID);
    }

    private String getSipxFreeswitchAddressAndPort() {
        SipxFreeswitchService service = getSipxFreeswitchService();
        String host;
        if (service.getAddresses().size() > 1) {
            // HACK: this assumes that one of the freeswitch instances runs on a primary location
            // (but that neeeds to be true in order for MOH to work anyway)
            host = service.getLocationsManager().getPrimaryLocation().getAddress();
        } else {
            host = service.getAddress();
        }

        return host + ":" + service.getFreeswitchSipPort();
    }
}
