/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.backup;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.backup.BackupPlan;
import org.sipfoundry.sipxconfig.backup.ManualRestore;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class RestoreForm extends BaseComponent implements PageBeginRenderListener {

    @Parameter(required = true)
    public abstract void setBackupPlan(BackupPlan plan);

    public abstract BackupPlan getBackupPlan();

    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selections);

    @InjectObject("spring:manualRestore")
    public abstract ManualRestore getManualRestore();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }

    public void restore() {
        // TODO
        // getManualRestore().restoreFromStage();
        getValidator().recordSuccess("Need to implement");
    }
}
