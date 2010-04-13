/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components.selection;

public interface OptionAdapter<T> {
    Object getValue(T option, int index);

    String getLabel(T option, int index);

    String squeezeOption(T option, int index);
}
