/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import static org.apache.commons.lang.StringUtils.join;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.dialplan.CallTag;

public class CdrSearch {
    public static final Map<String, String> ORDER_TO_COLUMN;
    public static final String OPEN_PARANTHESIS = "(";
    public static final String SQM_CLOSED_PARANTHESIS = "')"; // SQM=single quotation mark
    public static final String CLOSED_PARANTHESIS = ")"; //  Closed paranthesis
    public static final String AND = " AND ";
    public static final String OR = " OR ";
    public static final String SINGLE_QUOTE = "'";
    public static final String LIKE_WITH_WILDCARD = " LIKE '%";
    public static final String EQUALS_SQM = "='";

    static {
        Map<String, String> map = new HashMap<String, String>();
        map.put("caller", CdrManagerImpl.CALLER_AOR);
        map.put("callee", CdrManagerImpl.CALLEE_AOR);
        map.put("startTime", CdrManagerImpl.START_TIME);
        map.put("duration", CdrManagerImpl.END_TIME + " - " + CdrManagerImpl.CONNECT_TIME);
        map.put("termination", CdrManagerImpl.TERMINATION);
        ORDER_TO_COLUMN = map;
    }

    public enum Mode {
        NONE, CALLER, CALLEE, ANY
    }

    private Mode m_mode = Mode.NONE;
    private String[] m_term = new String[] {
        ""
    };
    private String m_order;
    private boolean m_ascending = true;

    public void setMode(Mode mode) {
        if (mode == null) {
            // XCF-1767 - IE lets you select disabled options, which have no value
            // and this method gets null mode. treat it as default case, otherwise
            // we get NPE trying to create SQL and clearly not a valid option for
            // this object.
            m_mode = Mode.NONE;
        } else {
            m_mode = mode;
        }
    }

    public Mode getMode() {
        return m_mode;
    }

    public void setTerm(String[] term) {
        m_term = term;
    }

    public String[] getTerm() {
        return m_term;
    }

    public void setOrder(String order, boolean ascending) {
        m_order = order;
        m_ascending = ascending;
    }

    private void appendSearchTermSql(StringBuilder sql, String call) {
        List<String> sqlList = new ArrayList<String>();
        for (String name : m_term) {
            sqlList.add(call + " LIKE '%<sip:" + name + "@%>'");
        }
        sql.append(join(sqlList.toArray(), OR));
    }

    private void appendCallerSql(StringBuilder sql) {
        sql.append(OPEN_PARANTHESIS);
        appendSearchTermSql(sql, CdrManagerImpl.CALLER_AOR);
        sql.append(AND);
        sql.append(CdrManagerImpl.CALLER_INTERNAL);
        sql.append("=true)");
    }

    private void appendCalleeSql(StringBuilder sql) {
        sql.append(OPEN_PARANTHESIS);
        appendSearchTermSql(sql, CdrManagerImpl.CALLEE_AOR);
        sql.append(AND);
        appendCalleeInternalRouteSql(sql);
        sql.append(CLOSED_PARANTHESIS);
    }

    private void appendCalleeInternalRouteSql(StringBuilder sql) {
        sql.append(OPEN_PARANTHESIS);
        sql.append(CdrManagerImpl.CALLEE_ROUTE);
        sql.append(LIKE_WITH_WILDCARD);
        sql.append(CallTag.INT);
        sql.append(SINGLE_QUOTE);
        sql.append(OR);
        sql.append(CdrManagerImpl.CALLEE_ROUTE);
        sql.append(LIKE_WITH_WILDCARD);
        sql.append(CallTag.AA);
        sql.append(SINGLE_QUOTE);
        sql.append(OR);
        sql.append(CdrManagerImpl.CALLEE_ROUTE);
        sql.append(LIKE_WITH_WILDCARD);
        sql.append(CallTag.VM);
        sql.append(SINGLE_QUOTE);
        sql.append(OR);
        sql.append(CdrManagerImpl.CALLEE_ROUTE);
        sql.append(LIKE_WITH_WILDCARD);
        sql.append(CallTag.PAGE);
        sql.append(SINGLE_QUOTE);
        sql.append(OR);
        sql.append(CdrManagerImpl.CALLEE_ROUTE);
        sql.append(LIKE_WITH_WILDCARD);
        sql.append(CallTag.PARK);
        sql.append(SINGLE_QUOTE);
        sql.append(OR);
        sql.append(CdrManagerImpl.CALLEE_ROUTE);
        sql.append(LIKE_WITH_WILDCARD);
        sql.append(CallTag.DPUP);
        sql.append(SINGLE_QUOTE);
        sql.append(OR);
        sql.append(CdrManagerImpl.CALLEE_ROUTE);
        sql.append(" IS NULL");
        sql.append(CLOSED_PARANTHESIS);
    }

    public boolean appendGetSql(StringBuilder sql) {
        switch (m_mode) {
        case CALLER:
            sql.append(AND);
            appendCallerSql(sql);
            break;
        case CALLEE:
            sql.append(AND);
            appendCalleeSql(sql);
            break;
        case ANY:
            sql.append(AND);
            sql.append(OPEN_PARANTHESIS);
            appendCallerSql(sql);
            sql.append(OR);
            appendCalleeSql(sql);
            sql.append(CLOSED_PARANTHESIS);
            break;
        default:
            return false;
        }
        return true;
    }

    public boolean appendOrderBySql(StringBuilder sql) {
        String column = orderToColumn();
        if (column == null) {
            return false;
        }
        sql.append(" ORDER BY ");
        sql.append(column);
        sql.append(' ');
        sql.append(m_ascending ? "ASC" : "DESC");
        return true;
    }

    public boolean isSearch() {
        return m_mode != Mode.NONE;
    }

    private String orderToColumn() {
        if (m_order == null) {
            return CdrManagerImpl.START_TIME;
        }
        return ORDER_TO_COLUMN.get(m_order);
    }
}
