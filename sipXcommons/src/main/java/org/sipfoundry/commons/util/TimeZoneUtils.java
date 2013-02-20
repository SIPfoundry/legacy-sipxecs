package org.sipfoundry.commons.util;

import java.util.Date;

import org.joda.time.DateTime;
import org.joda.time.DateTimeZone;
import org.joda.time.LocalDateTime;

public class TimeZoneUtils {

    public static Date convertJodaTimezone(LocalDateTime date, String srcTz, String destTz) {
        DateTime srcDateTime = date.toDateTime(DateTimeZone.forID(srcTz));
        DateTime dstDateTime = srcDateTime.withZone(DateTimeZone.forID(destTz));
        return dstDateTime.toLocalDateTime().toDateTime().toDate();
    }
}
