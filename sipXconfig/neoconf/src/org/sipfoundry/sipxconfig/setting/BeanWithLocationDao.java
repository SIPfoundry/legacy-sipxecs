/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.commserver.Location;

public interface BeanWithLocationDao<T> extends DataObjectSource<BeanWithSettings> {

    public T findOne(Location location);

    public Collection<T> findAll();

    public Collection<T> findAll(Location location);

    public void upsert(T object);

}
