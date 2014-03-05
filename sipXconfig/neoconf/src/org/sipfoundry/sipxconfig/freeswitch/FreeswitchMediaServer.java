/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch;


import java.util.Formatter;
import java.util.Map;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigException;
import org.sipfoundry.sipxconfig.dialplan.CallDigits;
import org.sipfoundry.sipxconfig.dialplan.MappingRule;
import org.sipfoundry.sipxconfig.dialplan.MediaServer;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.permission.PermissionName;

/**
 * AutoAttendant and Voicemail build on top of the freeswitch media server.
 *
 * A better name for this class would be SipxIvrMediaServer.
 *
 * Voicemail URIs:
 *
 * <pre>
 * Deposit: sip:IVR@{vm server}:15060;mailbox={mailbox};action=deposit;locale={locale}
 * Retrieve: sip:IVR@{vm server}:15060;action=retrieve;locale={locale}
 * </pre>
 *
 */
public class FreeswitchMediaServer extends MediaServer {
    private static final String USER_PART = "IVR";
    private AddressManager m_addressManager;

    @Override
    public String getDigitStringForOperation(Operation operation, CallDigits userDigits) {
        return USER_PART;
    }

    @Override
    public String getHeaderParameterStringForOperation(Operation operation, CallDigits userDigits) {
        return null;
    }

    @Override
    protected String getUriParameterStringForOperation(Operation operation, CallDigits userDigits,
            Map<String, String> additionalParams) {
        StringBuilder params = new StringBuilder();
        Formatter f = new Formatter(params);

        if (operation == Operation.VoicemailDeposit) {
            f.format("mailbox={%s};action=deposit", userDigits.getName());
        } else if (operation == Operation.FaxForward) {
            f.format("mailbox={%s};action=faxrx", userDigits.getName());
        } else if (operation == Operation.LiveAttendantManagement) {
            f.format("dialed={%s};action=aamgmt", userDigits.getName());
        } else {
            params.append("action=retrieve");
        }
        appendLocale(f);
        return params.toString();
    }

    @Override
    public String getName() {
        return "sipXivr";
    }

    @Override
    public PermissionName getPermissionName() {
        return PermissionName.FREESWITH_VOICEMAIL;
    }

    @Override
    public String buildAttendantUrl(String attendantName) {
        StringBuilder params = new StringBuilder("action=autoattendant");
        Formatter f = new Formatter(params);
        f.format(";schedule_id=%s", attendantName);
        appendLocale(f);
        return MappingRule.buildUrl(USER_PART, getHostname(Operation.Autoattendant), params.toString(), null, null);
    }

    @Override
    public String getHostname(Operation operation) {
        if (getLocation() != null) {
            Address addr = m_addressManager.getSingleAddress(Ivr.SIP_ADDRESS, getLocation());
            if (addr != null) {
                return addr.getAddress();
            }
        }
        return getRegularHostname();
    }

    public String getRegularHostname() {
        Address fs = m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        if (fs == null) {
            throw new ConfigException("Freeswitch is not enabled but media services is required");
        }
        return fs.addressColonPort();
    }

    private void appendLocale(Formatter f) {
        String language = getLanguage();
        if (language != null) {
            f.format(";locale=%s", language);
        }
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }
}
