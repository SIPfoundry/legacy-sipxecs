/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.firewall;

import java.util.Collection;
import java.util.List;

public interface CallRateManager {

    public List<CallRateRule> getCallRateRules();

    public CallRateRule getCallRateRule(Integer id);

    public void saveCallRateRule(CallRateRule rate);

    public void saveCallRateRules(List<CallRateRule> rate);

    public void deleteCallRateRules(Collection<CallRateRule> rules);
}
