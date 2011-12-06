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

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface PagingContext extends DialingRuleProvider, AliasOwner {
    public static LocationFeature FEATURE = new LocationFeature("page");
    public static AddressType SIP_TCP = new AddressType("pageTcp");

    String getPagingPrefix();

    void setPagingPrefix(String prefix);

    String getSipTraceLevel();

    void setSipTraceLevel(String traceLevel);

    PagingServer getPagingServer();

    List<PagingGroup> getPagingGroups();

    PagingGroup getPagingGroupById(Integer id);

    void deletePagingGroupsById(Collection<Integer> groupsIds);

    void savePagingGroup(PagingGroup group);

    void clear();

}
