package org.sipfoundry.openfire.plugin.job;

import java.io.Serializable;

public interface Job extends Serializable {
    void process();
}
