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
