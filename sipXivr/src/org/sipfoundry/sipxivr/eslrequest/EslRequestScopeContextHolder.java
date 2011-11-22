/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
