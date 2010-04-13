/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;

public abstract class AcdStatsPanel extends BaseComponent implements PageBeginRenderListener {
//    public abstract String getAcdQueueUri();
//    public abstract AcdStatistics getAcdStatistics();
//    public abstract String getSelectedAcdQueueUri();

    public void pageBeginRender(PageEvent event) {
//        if (event.getRequestCycle().isRewinding()) {
//            AcdStatistics context = getAcdStatistics();
//            String queueUri = getAcdQueueUri();
//            if (queueUri == null) {
//                queueUri = getSelectedAcdQueueUri();
//            }
//            context.setQueueUri(queueUri);
//        }
    }
}
