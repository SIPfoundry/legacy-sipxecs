/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

/**
 * A LegListener recieves LegEvent's via the
 * onEvent() method.
 *
 * @author Woof!
 *
 */
public interface LegListener
{
   public boolean onEvent(LegEvent event);
}
