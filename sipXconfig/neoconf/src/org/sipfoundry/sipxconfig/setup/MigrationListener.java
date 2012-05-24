/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.setup;

/**
 * Implement to be called when any of these conditions are true:
 *  1.) sipXconfig has just finished apply sql patches and is now allowing java code
 *      to further repair data.
 *
 *  In order to decide when to do what and when to not repeat tasks, use SetupManager interface
 *  to record boolean flags.  Flag can mean, if it exists, do something or it can mean if it doesn't
 *  exist do something.
 *
 *  Use as many flags as you want, but be sure to pick unique ids.
 */
public interface MigrationListener {

    public void migrate(SetupManager manager);

}
