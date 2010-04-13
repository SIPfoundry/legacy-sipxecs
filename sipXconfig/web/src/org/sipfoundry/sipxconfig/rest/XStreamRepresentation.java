/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

/*
 * The content of this file is a slightly modified copy of XstremRepresnetation.
 * It's been added to restlet 2.0: we cannot use 2.0 for now.
 *
 * Copyright 2005-2009 Noelios Technologies.
 *
 * The contents of this file are subject to the terms of one of the following
 * open source licenses: LGPL 3.0 or LGPL 2.1 or CDDL 1.0 or EPL 1.0 (the
 * "Licenses"). You can select the license that you prefer but you may not use
 * this file except in compliance with one of these Licenses.
 *
 */

package org.sipfoundry.sipxconfig.rest;

import java.io.IOException;
import java.io.OutputStream;

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.io.HierarchicalStreamDriver;
import com.thoughtworks.xstream.io.json.JettisonMappedXmlDriver;
import com.thoughtworks.xstream.io.json.JsonHierarchicalStreamDriver;
import com.thoughtworks.xstream.io.xml.DomDriver;
import com.thoughtworks.xstream.mapper.MapperWrapper;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.data.MediaType;
import org.restlet.resource.OutputRepresentation;
import org.restlet.resource.Representation;
import org.sipfoundry.sipxconfig.common.XstreamFieldMapper;

/**
 * Representation based on the XStream library. It can serialize and deserialize automatically in
 * either JSON or XML.
 *
 */
public class XStreamRepresentation<T> extends OutputRepresentation {
    private static final Log LOG = LogFactory.getLog(XStreamRepresentation.class);

    private final Class< ? extends HierarchicalStreamDriver> m_jsonDriverClass;
    private final Class< ? extends HierarchicalStreamDriver> m_xmlDriverClass;

    private final T m_object;

    private final Representation m_representation;

    private XStream m_xstream;

    private final boolean m_parse;

    /**
     * Constructor.
     *
     * @param mediaType The target media type.
     * @param object The object to format.
     */
    public XStreamRepresentation(MediaType mediaType, T object) {
        super(mediaType);
        m_object = object;
        m_representation = null;
        m_jsonDriverClass = JsonHierarchicalStreamDriver.class;
        m_xmlDriverClass = DomDriver.class;
        m_parse = false;
    }

    /**
     * Constructor.
     *
     * @param representation The representation to parse.
     */
    public XStreamRepresentation(Representation representation) {
        super(representation.getMediaType());
        m_object = null;
        m_representation = representation;
        m_jsonDriverClass = JettisonMappedXmlDriver.class;
        m_xmlDriverClass = DomDriver.class;
        m_parse = true;
    }

    protected XStream newXStreamInstance(Class< ? extends HierarchicalStreamDriver> driver) {
        try {
            return new XStream(driver.newInstance()) {
                @Override
                protected MapperWrapper wrapMapper(MapperWrapper next) {
                    return new XstreamFieldMapper(next);
                }
            };
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InstantiationException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Override to configure xstream aliases
     *
     * @param xstream XStream instance to be configured
     */
    protected void configureXStream(XStream xstream) {
    }

    /**
     * Override to declare collections as implicit: it's needed for JSON parsing.
     *
     * @param xstream XStream instance to be configured
     */
    protected void configureImplicitCollections(XStream xstream) {
    }

    /**
     * Creates an XStream object based on a media type. By default, it creates a
     * {@link JsonHierarchicalStreamDriver} or a {@link DomDriver}.
     *
     * @param mediaType The serialization media type.
     * @return The XStream object.
     */
    protected XStream createXstream(MediaType mediaType) {
        boolean json = MediaType.APPLICATION_JSON.isCompatible(mediaType);

        XStream xs = newXStreamInstance(json ? m_jsonDriverClass : m_xmlDriverClass);
        xs.setMode(XStream.NO_REFERENCES);
        configureXStream(xs);
        if (json && m_parse) {
            // if parsing JSON additional config is needed
            configureImplicitCollections(xs);
        }
        return xs;
    }

    public T getObject() {
        T result = null;

        if (this.m_object != null) {
            result = this.m_object;
        } else if (this.m_representation != null) {
            try {
                result = (T) getXstream().fromXML(this.m_representation.getStream());
            } catch (IOException e) {
                LOG.warn("Unable to parse the object with XStream.", e);
            }
        }

        return result;
    }

    /**
     * @return lazily created XStream object
     */
    private XStream getXstream() {
        if (m_xstream == null) {
            m_xstream = createXstream(getMediaType());
        }
        return m_xstream;
    }

    @Override
    public void write(OutputStream outputStream) throws IOException {
        if (m_representation != null) {
            m_representation.write(outputStream);
        } else if (m_object != null) {
            getXstream().toXML(m_object, outputStream);
        }
    }
}
