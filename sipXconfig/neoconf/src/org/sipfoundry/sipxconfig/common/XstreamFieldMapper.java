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
 * <pre>
 *   xstream = new XStream() {
 *       protected MapperWrapper wrapMapper(MapperWrapper next) {
 *           return new XstreamFieldMapper(next);
 *       }
 *   };
 *   </pre>
 *
 * @author dhubler
 *
 */
public class XstreamFieldMapper  extends MapperWrapper {
    private static final String CODING_CONVENTION_FIELD_PREFIX = "m_";
    
    public XstreamFieldMapper(Mapper wrapped) {
        super(wrapped);
    }

    public String serializedMember(Class type, String memberNameCandidate) {
        String memberName = memberNameCandidate;
        if (memberNameCandidate.startsWith(CODING_CONVENTION_FIELD_PREFIX)) {
            memberName = memberNameCandidate.substring(2);
        }
        return super.serializedMember(type, memberName);
    }

    public String realMember(Class type, String serialized) {
        String fieldName = super.realMember(type, serialized);
        // Not very efficient or elegant, but enough to get the point across.
        // Luckily the CachingMapper will ensure this is only ever called once per field per class.
        try {
            type.getDeclaredField(CODING_CONVENTION_FIELD_PREFIX + fieldName);
            return CODING_CONVENTION_FIELD_PREFIX + fieldName;
        } catch (NoSuchFieldException e) {
            return fieldName;
        }
    }
}
