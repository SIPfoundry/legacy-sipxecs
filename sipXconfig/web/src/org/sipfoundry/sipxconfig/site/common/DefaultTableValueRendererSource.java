/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import java.text.DateFormat;
import java.text.Format;
import java.util.Date;
import java.util.Locale;

import org.apache.tapestry.IRender;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableModelSource;
import org.apache.tapestry.contrib.table.model.ITableRendererSource;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumn;
import org.apache.tapestry.valid.RenderString;
import org.sipfoundry.sipxconfig.common.SqlInterval;
import org.sipfoundry.sipxconfig.components.MillisDurationFormat;

/**
 * Influence default display behaviour of data. For more customized results use ColumnValueBlocks.
 */
public class DefaultTableValueRendererSource implements ITableRendererSource {
    private MillisDurationFormat m_durationFormat = new MillisDurationFormat();
    private ITableRendererSource m_default = SimpleTableColumn.DEFAULT_VALUE_RENDERER_SOURCE;

    public DefaultTableValueRendererSource() {
        m_durationFormat.setMaxField(2);
    }

    public IRender getRenderer(IRequestCycle objCycle, ITableModelSource objSource,
            ITableColumn objColumn, Object objRow) {
        Object value = ((SimpleTableColumn) objColumn).getColumnValue(objRow);
        IRender render = getRender(value, objCycle.getPage().getLocale());
        if (render == null) {
            render = m_default.getRenderer(objCycle, objSource, objColumn, objRow);
        }

        return render;
    }

    protected IRender getRender(Object value, Locale locale) {
        IRender render = null;
        if (value instanceof SqlInterval) {
            long millis = ((SqlInterval) value).getMillisecs();
            m_durationFormat.setLocale(locale);
            String sInterval = m_durationFormat.format(millis);
            render = new RenderString(sInterval);
        } else if (value instanceof Date) {
            Format format = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT,
                    locale);
            String sValue = format.format(value);
            render = new RenderString(sValue);
        }

        return render;
    }
}
