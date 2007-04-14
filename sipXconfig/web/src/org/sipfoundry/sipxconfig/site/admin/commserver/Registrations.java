/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.apache.commons.lang.time.DateUtils;
import org.apache.tapestry.IRender;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableModelSource;
import org.apache.tapestry.contrib.table.model.ITableRendererSource;
import org.apache.tapestry.contrib.table.model.simple.ITableColumnEvaluator;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumn;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.RenderString;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;

public abstract class Registrations extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "Registrations";

    private static final String EXPIRES_COLUMN = "expires";

    private long m_startRenderingTime;
    
    public void pageBeginRender(PageEvent event_) {
        m_startRenderingTime = System.currentTimeMillis() / DateUtils.MILLIS_PER_SECOND;
    }
    
    public long getStartTime() {
        return m_startRenderingTime;
    }

    public ITableColumn getExpiresColumn() {
        ITableColumnEvaluator eval = new ExpireTimeEvaluator();
        ExpireTimeRendererSource rendererSource = new ExpireTimeRendererSource(
                getMessages().getMessage("status.expired"));
        SimpleTableColumn column = new SimpleTableColumn(EXPIRES_COLUMN,
                getMessages().getMessage(EXPIRES_COLUMN), eval, true);
        column.setValueRendererSource(rendererSource);
        return column;
    }

    private static class ExpireTimeRendererSource implements ITableRendererSource {
        private IRender m_expiredRenderer;

        public ExpireTimeRendererSource(String msg) {
            m_expiredRenderer = new RenderString(msg);
        }

        public IRender getRenderer(IRequestCycle objCycle_, ITableModelSource objSource_,
                ITableColumn objColumn, Object objRow) {
            SimpleTableColumn objSimpleColumn = (SimpleTableColumn) objColumn;

            Long expired = (Long) objSimpleColumn.getColumnValue(objRow);
            if (expired.longValue() > 0) {
                return new RenderString(expired.toString());
            }
            return m_expiredRenderer;
        }
    }

    private class ExpireTimeEvaluator implements ITableColumnEvaluator {
        public Object getColumnValue(ITableColumn objColumn_, Object objRow) {
            RegistrationItem item = (RegistrationItem) objRow;
            long l = item.timeToExpireAsSeconds(m_startRenderingTime);
            return new Long(l);
        }
    }
}
