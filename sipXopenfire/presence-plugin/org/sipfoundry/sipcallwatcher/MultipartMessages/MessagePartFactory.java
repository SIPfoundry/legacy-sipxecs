/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher.MultipartMessages;


/**
 * Interface for a factory capable of constructing MessagePart-derived
 * object for the supplied message part body.
 *
 */
public interface MessagePartFactory 
{
	MessagePart createMessagePart( String messageBody ) throws Exception;
}
