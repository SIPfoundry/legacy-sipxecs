/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import java.io.Serializable;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface ConferenceBridgeContext extends AliasOwner {
    static final LocationFeature FEATURE = new LocationFeature("conference");
    String CONTEXT_BEAN_NAME = "conferenceBridgeContext";

    List<Bridge> getBridges();

    void saveBridge(Bridge bridge);

    void saveConference(Conference conference);

    /**
     * Check whether the conference is valid and can be stored. If the conference is OK, then
     * return. If it's not OK, then throw UserException.
     */
    void validate(Conference conference);

    Bridge newBridge();

    void removeBridge(Bridge bridge);

    Conference newConference();

    void removeConferences(Collection<Integer> conferencesIds);

    Bridge loadBridge(Serializable serverId);

    Bridge getBridgeByServer(String hostname);

    List<Conference> getAllConferences();

    Conference loadConference(Serializable id);

    Conference findConferenceByName(String name);

    List<Conference> findConferencesByOwner(User owner);

    String getAddressSpec(Conference conference);

    void clear();

    List<Conference> filterConferences(final Integer bridgeId, final Integer ownerGroupId);

    List<Conference> searchConferences(final String searchTerm);

    int countFilterConferences(final Integer bridgeId, final Integer ownerGroupId);

    List<Conference> filterConferencesByPage(final Integer bridgeId, final Integer ownerGroupId, int firstRow,
            int pageSize, String[] orderBy, boolean orderAscending);
}
