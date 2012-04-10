/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
