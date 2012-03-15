/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxivr.eslrequest;

public class EslRequestScopeContextHolder {

    private static final ThreadLocal<EslRequestScopeAttributes> eslRequestScopeAttributesHolder = new InheritableThreadLocal<EslRequestScopeAttributes>() {
        protected EslRequestScopeAttributes initialValue() {
            return new EslRequestScopeAttributes();
        }
    };

    public static EslRequestScopeAttributes getEslRequestScopeAttributes() {
        return eslRequestScopeAttributesHolder.get();
    }

    public static void setEslRequestScopeAttributes(EslRequestScopeAttributes accessor) {
        EslRequestScopeContextHolder.eslRequestScopeAttributesHolder.set(accessor);
    }

    public static EslRequestScopeAttributes currentEslRequestScopeAttributes() throws IllegalStateException {
        EslRequestScopeAttributes accessor = eslRequestScopeAttributesHolder.get();

        if (accessor == null) {
            throw new IllegalStateException("No thread scoped attributes.");
        }

        return accessor;
    }

}
