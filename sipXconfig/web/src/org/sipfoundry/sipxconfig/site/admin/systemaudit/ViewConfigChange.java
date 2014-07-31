/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
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
 *
 */

package org.sipfoundry.sipxconfig.site.admin.systemaudit;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChange;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeAction;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeContext;

public abstract class ViewConfigChange extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "admin/systemaudit/ViewConfigChange";

    @Persist
    public abstract ConfigChange getConfigChange();
    public abstract void setConfigChange(ConfigChange configChange);

    @Persist
    public abstract String getConfigChangeType();
    public abstract void setConfigChangeType(String configChangeType);

    @Persist
    public abstract String getConfigChangeAction();
    public abstract void setConfigChangeAction(String configChangeAction);

    @Persist
    public abstract String getDetails();
    public abstract void setDetails(String details);

    @InjectObject(value = "spring:configChangeContext")
    public abstract ConfigChangeContext getConfigChangeContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public void pageBeginRender(PageEvent event_) {
    }

    public IBasicTableModel getTableModel() {
        return new ConfigChangeValueTableModel(getConfigChangeContext(), getConfigChange());
    }

    public String cancel() {
        return SystemAuditHistory.PAGE;
    }

    public boolean getHasConfigChangeValues() {
        return !getConfigChange().getValues().isEmpty();
    }

    public String getQuickHelp() {
        if (getConfigChange().getConfigChangeAction().equals(ConfigChangeAction.MODIFIED)) {
            return getMessages().getMessage("systemaudit.modifiedaction.quick.help");
        } else {
            return getMessages().getMessage("systemaudit.quick.help");
        }
    }

}
