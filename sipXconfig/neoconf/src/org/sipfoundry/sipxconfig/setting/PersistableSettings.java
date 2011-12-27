/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

public abstract class PersistableSettings extends BeanWithSettings {

    public abstract String getBeanId();

    /**
     * Should not be called. Only used to comply w/hibernate requirement of setter.
     * @param ignore
     */
    public void setBeanId(String ignore) {
    }
}
