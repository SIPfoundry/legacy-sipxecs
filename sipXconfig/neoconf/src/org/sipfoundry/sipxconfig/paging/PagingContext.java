/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import java.util.List;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface PagingContext extends DialingRuleProvider, AliasOwner {
    public static LocationFeature FEATURE = new LocationFeature("page");
    public static AddressType SIP_TCP = new AddressType("pageTcp");
    public static AddressType SIP_UDP = new AddressType("pageUdp");
    public static AddressType SIP_TLS = new AddressType("pageTls");
    public static AddressType RTP_PORT = new AddressType("pageRtp");

    PagingSettings getSettings();

    void saveSettings(PagingSettings settings);

    List<PagingGroup> getPagingGroups();

    PagingGroup getPagingGroupById(Integer id);

    void savePagingGroup(PagingGroup group);

    void clear();
}
