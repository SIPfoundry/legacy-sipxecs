/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.text.DateFormat;
import java.util.Date;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IRender;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableModelSource;
import org.apache.tapestry.contrib.table.model.ITableRendererSource;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumn;
import org.apache.tapestry.valid.RenderString;

public final class DateTableRendererSource implements ITableRendererSource {
    private DateFormat m_format;

    public DateTableRendererSource(DateFormat format) {
        m_format = format;
    }

    public IRender getRenderer(IRequestCycle objCycle_, ITableModelSource objSource_,
            ITableColumn objColumn, Object objRow) {
        SimpleTableColumn objSimpleColumn = (SimpleTableColumn) objColumn;

        Object objValue = objSimpleColumn.getColumnValue(objRow);
        if (objValue == null) {
            return new RenderString("&nbsp;", true);
        }

        return new RenderString(formatDate((Date) objValue));
    }

    private String formatDate(Date date) {
        if (date == null) {
            return StringUtils.EMPTY;
        }
        return m_format.format(date);
    }
}
