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

/**
 * Used by sipX table component to influence rendering rows.
 */
public interface RowInfo {

    /**
     * If a row (or all rows) aren't selectable, you can use this
     */
    public static final RowInfo UNSELECTABLE = new RowInfo() {

        public Object getSelectId(Object row) {
            return null;
        }

        public boolean isSelectable(Object row) {
            return false;
        }

    };

    /**
     * @return true if checkbox for row selection is to be rendered, false otherwise
     */
    boolean isSelectable(Object row);

    /**
     * Provide custom primary key conversions. primary for table selection
     */
    Object getSelectId(Object row);
}
