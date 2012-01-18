/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common.event;

/**
 * Attach this to method that normally triggers a DaoEvent to be published. Events are
 * automatically published with methods when application enters a method on a Dao interface that
 * is named save*(Object ...) or delete*(Object ...).
 *
 * There are certain situations where you do not want an event to be published for example if
 * there is another interface that will be triggering the event.
 *
 * Example:
 *   saveFoo(Object foo) {
 *      delegate.saveFoo(foo);
 *   }
 *  in this case 2 events would be triggered
 *
 */
public @interface SupressDaoEvent {

}
