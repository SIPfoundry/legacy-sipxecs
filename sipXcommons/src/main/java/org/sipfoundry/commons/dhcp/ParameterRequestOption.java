/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dhcp;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.LinkedList;

import static org.sipfoundry.commons.dhcp.DHCPOption.Code.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ParameterRequestOption extends DHCPOption {
    private LinkedList<Code> parameterList;

    public ParameterRequestOption() {
        super.setCode(PARAMETER_REQUEST);
    }

    public ParameterRequestOption(Code mandatoryParameter, Code... additionalParameters) {
        super.setCode(PARAMETER_REQUEST);
        parameterList = new LinkedList<Code>();
        super.setLength(1 + additionalParameters.length);
        parameterList.add(mandatoryParameter);
        for (Code parameter : additionalParameters) {
            parameterList.add(parameter);
        }
    }

    public LinkedList<Code> getParameterList() {
        return parameterList;
    }

    public void addParameter(Code parameter) {
        if (parameterList == null) {
            parameterList = new LinkedList<Code>();
        }
        parameterList.add(parameter);
        super.setLength(super.getLength() + 1);
    }

    public String toString() {
        String parameters = "";
        for (Code parameter : parameterList) {
            if (parameters.length() > 0) {
                parameters += ", ";
            }
            parameters += parameter.toInt();
        }
        return parameters;
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        if (parameterList != null) {
            try {
                dataStream.writeByte(super.getCode().toInt());
                dataStream.writeByte(super.getLength());
                for (Code parameter : parameterList) {
                    dataStream.writeByte(parameter.toInt());
                }
            } catch (IOException e) {
                System.err.println(e);
            }
        }

        return byteStream.toByteArray();
    }

    public void unmarshal(DataInputStream dataStream) throws IOException {
        int length = dataStream.readByte() & 0xFF;
        if (length < 1) {
            throw new IOException();
        }
        super.setLength(length);
        parameterList = new LinkedList<Code>();
        while (length > 0) {
            parameterList.add(Code.toEnum(dataStream.readByte() & 0xFF));
            --length;
        }
    }

}
