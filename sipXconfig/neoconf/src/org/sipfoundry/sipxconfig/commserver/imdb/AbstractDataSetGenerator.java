/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.commserver.imdb;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Replicable;

import com.mongodb.DBObject;

public abstract class AbstractDataSetGenerator {
    private CoreContext m_coreContext;

    public abstract boolean generate(Replicable entity, DBObject top);

    protected abstract DataSet getType();

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    protected CoreContext getCoreContext() {
        return m_coreContext;
    }

    /**
     * @return SIP domain - if not set uses m_coreContext to retrieve domain
     */
    protected String getSipDomain() {
        return m_coreContext.getDomainName();
    }

    protected void putOnlyIfNotNull(DBObject obj, String propName, Object prop) {
        if (prop instanceof String) {
            if (StringUtils.isNotBlank((String) prop)) {
                obj.put(propName, prop);
            }
        } else if (prop != null) {
            obj.put(propName, prop);
        } else {
            obj.removeField(propName);
        }
    }

}
