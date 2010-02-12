package org.sipfoundry.commons.configstore;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)

public @interface UpdateNotifier {
    String[] value();
}
