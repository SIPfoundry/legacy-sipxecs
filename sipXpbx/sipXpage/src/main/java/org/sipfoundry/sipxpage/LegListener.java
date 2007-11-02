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
