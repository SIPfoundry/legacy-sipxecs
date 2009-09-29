/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

/**
 * Most objects save save to database, and possible other types as well, can
 * have an id that uniquely maps to them.  This captures that id for enviroments
 * that can leverage that, for example web interface that wants to draw checkboxes
 * for objects id selection.
 */
public interface PrimaryKeySource {

    public Object getPrimaryKey();

}
