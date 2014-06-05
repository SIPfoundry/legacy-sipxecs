package org.sipfoundry.openfire.sync.job;

import java.io.Serializable;

public interface Job extends Serializable {
    void process();
}
