/*
 *
 *
 * Copyright (c) 2012 Karel Electronics Corp. All rights reserved.
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
 *
 */
package org.sipfoundry.sipxconfig.common;

/**
 * use when 2 configured extensions for the same entity are the same;
 * for instance alias and did, or alias ane extension.
 */
public class SameExtensionException extends UserException {
    private static final String ERROR = "&error.sameExtension";

    /*
     * use: SameExtensionException("alias", "did")
     */
    public SameExtensionException(String ext1Type, String ext2Type) {
        super(ERROR, ext1Type, ext2Type);
    }
}
