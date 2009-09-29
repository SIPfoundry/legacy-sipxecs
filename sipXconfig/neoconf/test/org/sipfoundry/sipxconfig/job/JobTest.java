/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.job;

import java.util.Date;

import junit.framework.TestCase;

public class JobTest extends TestCase {

    public void testSuccess() {
        Job job = new Job("xxxx");
        assertEquals("xxxx", job.getName());

        assertEquals(JobStatus.SCHEDULED, job.getStatus());

        job.start();
        assertEquals(JobStatus.IN_PROGRESS, job.getStatus());

        Date during = new Date();

        job.success();
        assertEquals(JobStatus.COMPLETED, job.getStatus());

        assertFalse(job.getStart().after(during));
        assertFalse(job.getStop().before(during));

        assertEquals(0, job.getErrorMsg().length());
    }

    public void testFailure() {
        Job job = new Job();
        job.start();
        Exception e = new IllegalArgumentException("exception error");
        job.failure("some error", e);
        assertEquals(JobStatus.FAILED, job.getStatus());

        assertEquals("some error\nexception error", job.getErrorMsg());
    }

    public void testErrorMessage() {
        Job job = new Job();
        job.start();
        job.failure(null, new RuntimeException());
        assertNotNull(job.getErrorMsg());
    }

    public void testIllegalFailure() {
        Job job = new Job();
        job.start();
        job.success();

        try {
            job.failure(null, null);
            fail("Should have thrown illegal state exception");
        } catch (IllegalStateException e) {
            // ok
        }
    }

    public void testIllegalSuccess() {
        Job job = new Job();
        job.start();
        job.failure(null, null);

        try {
            job.success();
            fail("Should have thrown illegal state exception");
        } catch (IllegalStateException e) {
            // ok
        }
    }

    public void testNotStartedFailure() {
        Job job = new Job();
        try {
            job.failure(null, null);
            fail("Should have thrown illegal state exception");
        } catch (IllegalStateException e) {
            // ok
        }
    }

    public void testNotStartedSuccess() {
        Job job = new Job();
        try {
            job.success();
            fail("Should have thrown illegal state exception");
        } catch (IllegalStateException e) {
            // ok
        }
    }
}
