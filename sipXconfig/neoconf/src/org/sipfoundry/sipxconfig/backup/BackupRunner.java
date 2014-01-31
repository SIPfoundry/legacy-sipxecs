/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.backup;

import java.io.File;
import java.util.Collection;
import java.util.List;
import java.util.Map;

public interface BackupRunner {

    public boolean isInProgress();

    public Map<String, List<String>> list(File plan);

    public boolean backup(File plan);

    /**
     * Restore selected archives. a selection has this form: 201401301800/configuration.tar.gz
     * Year 2014, Month 01=January, Day 30, Hour 18, Minutes 00
     *
     * The restore occurs in two steps:
     * 1. - stage the archive (copy it from local backup path or download from FTP (depending of File plan content) into
     * restore temp path: var/sipxdata/tmp/restore/)
     * sample command:
     * sipx-archive --stage /backup.txt,201401301800/configuration.tar.gz
     * 2. - perform restore
     *      (correlate archive depending on location (move from tmp/restore/ into tmp/restore/location_id and restore)
     * sample restore command flow:
     * sipx-archive --restore /usr/local/sipx/var/sipxdata/tmp/poifiles/restore-1219980797yaml
     *   correlate
     *   restore: sipx-archive --restore-host 1 < /usr/local/sipx/var/sipxdata/tmp/poifiles/restore-1219980797yaml
     * @param plan
     * @param selections
     * @return
     */
    public boolean restore(File plan, Collection<String> selections);
}
