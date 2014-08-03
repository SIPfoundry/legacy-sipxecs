/**
 *
 *
 * Copyright (c) 2014 Karel, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;

import org.hibernate.Criteria;
import org.hibernate.EntityMode;
import org.hibernate.HibernateException;
import org.hibernate.criterion.CriteriaQuery;
import org.hibernate.criterion.Criterion;
import org.hibernate.engine.TypedValue;
import org.hibernate.type.CompositeType;
import org.hibernate.type.Type;
import org.hibernate.util.StringHelper;

public class InExpressionIgnoringCase implements Criterion {

    private static final String LEFT_P = " ( ";
    private static final String EMPTY_STRING = "";
    private static final String COMMA = ", ";
    private static final String IN = " in (";

    private final String m_propertyName;
    private final Object[] m_values;

    public InExpressionIgnoringCase(final String propertyName,
            final Object[] values) {
        this.m_propertyName = propertyName;
        this.m_values = values;
    }

    public String toSqlString(final Criteria criteria,
            final CriteriaQuery criteriaQuery) throws HibernateException {
        final String[] columns = criteriaQuery.findColumns(this.m_propertyName,
                criteria);
        final String[] wrappedLowerColumns = wrapLower(columns);
        if (criteriaQuery.getFactory().getDialect()
                .supportsRowValueConstructorSyntaxInInList()
                || columns.length <= 1) {

            String singleValueParam = StringHelper.repeat("lower(?), ",
                    columns.length - 1) + "lower(?)";
            if (columns.length > 1) {
                singleValueParam = '(' + singleValueParam + ')';
            }
            final String params = this.m_values.length > 0 ? StringHelper.repeat(
                    singleValueParam + COMMA, this.m_values.length - 1)
                    + singleValueParam : EMPTY_STRING;
            String cols = StringHelper.join(COMMA, wrappedLowerColumns);
            if (columns.length > 1) {
                cols = '(' + cols + ')';
            }
            return cols + IN + params + ')';
        } else {
            String cols = LEFT_P
                    + StringHelper
                            .join(" = lower(?) and ", wrappedLowerColumns)
                    + "= lower(?) ) ";
            cols = this.m_values.length > 0 ? StringHelper.repeat(cols + "or ",
                    this.m_values.length - 1) + cols : EMPTY_STRING;
            cols = LEFT_P + cols + " ) ";
            return cols;
        }
    }

    public TypedValue[] getTypedValues(final Criteria criteria,
            final CriteriaQuery criteriaQuery) throws HibernateException {
        final ArrayList<TypedValue> list = new ArrayList<TypedValue>();
        final Type type = criteriaQuery.getTypeUsingProjection(criteria,
                this.m_propertyName);
        if (type.isComponentType()) {
            final CompositeType actype = (CompositeType) type;
            final Type[] types = actype.getSubtypes();
            for (int j = 0; j < this.m_values.length; j++) {
                for (int i = 0; i < types.length; i++) {
                    final Object subval = this.m_values[j] == null ? null
                            : actype.getPropertyValues(this.m_values[j],
                                    EntityMode.POJO)[i];
                    list.add(new TypedValue(types[i], subval, EntityMode.POJO));
                }
            }
        } else {
            for (int j = 0; j < this.m_values.length; j++) {
                list.add(new TypedValue(type, this.m_values[j], EntityMode.POJO));
            }
        }
        return list.toArray(new TypedValue[list.size()]);
    }

    @Override
    public String toString() {
        return this.m_propertyName + IN + StringHelper.toString(this.m_values)
                + ')';
    }

    private String[] wrapLower(final String[] columns) {
        final String[] wrappedColumns = new String[columns.length];
        for (int i = 0; i < columns.length; i++) {
            wrappedColumns[i] = "lower(" + columns[i] + ")";
        }
        return wrappedColumns;
    }
}
