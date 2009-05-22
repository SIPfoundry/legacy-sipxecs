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

import com.thoughtworks.xstream.mapper.Mapper;
import com.thoughtworks.xstream.mapper.MapperWrapper;

/**
 * When using Xstream package to serialize or deserialize beans.
 *
 * <pre>
 * xstream = new XStream() {
 *     protected MapperWrapper wrapMapper(MapperWrapper next) {
 *         return new XstreamFieldMapper(next);
 *     }
 * };
 * </pre>
 *
 * @author dhubler
 *
 */
public class XstreamFieldMapper extends MapperWrapper {
    private static final String CODING_CONVENTION_FIELD_PREFIX = "m_";

    public XstreamFieldMapper(Mapper wrapped) {
        super(wrapped);
    }

    @Override
    public String serializedMember(Class type, String memberNameCandidate) {
        String memberName = memberNameCandidate;
        if (memberNameCandidate.startsWith(CODING_CONVENTION_FIELD_PREFIX)) {
            memberName = memberNameCandidate.substring(CODING_CONVENTION_FIELD_PREFIX.length());
        }
        return super.serializedMember(type, memberName);
    }

    @Override
    public String realMember(Class type, String serialized) {
        final String nakedField = super.realMember(type, serialized);
        // Check if prefixed version is declared for any members of the hierarchy.
        // CachingMapper will ensure this is only ever called once per field per class.
        String prefixedField = CODING_CONVENTION_FIELD_PREFIX + nakedField;
        for (Class k = type; k != null; k = k.getSuperclass()) {
            if (hasField(k, prefixedField)) {
                return prefixedField;
            }
        }

        return nakedField;
    }

    private static boolean hasField(Class type, String fieldName) {
        try {
            type.getDeclaredField(fieldName);
            return true;
        } catch (NoSuchFieldException e) {
            return false;
        }
    }

}
