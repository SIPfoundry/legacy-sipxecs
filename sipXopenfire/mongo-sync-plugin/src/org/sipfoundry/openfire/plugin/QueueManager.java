package org.sipfoundry.openfire.plugin;

import java.util.LinkedList;
import java.util.Queue;

import org.apache.log4j.Logger;
import org.sipfoundry.openfire.plugin.job.Job;

public class QueueManager {
    private static Logger logger = Logger.getLogger(QueueManager.class);

    // must preserve the order of events
    private static final Queue<Job> QUEUE;

    private static final JobWatcher WATCHER;

    static {
        QUEUE = initQueue();

        WATCHER = new JobWatcher();
        // give it a distinctive name, but keep the number
        WATCHER.setName(WATCHER.getName().replace("Thread", "watcher"));
        WATCHER.start();
    }

    public static void submitJob(Job j) {
        logger.debug(String.format("Submitting job %s", j));
        // try to avoid several nodes submitting the same change for processing
        if (!QUEUE.contains(j)) {
            QUEUE.add(j);
        } else {
            logger.debug(String.format("Job already queued: %s", j));
        }
    }

    public static void shutdown() {
        WATCHER.shutdown();
    }

    protected static Queue<Job> getQueue() {
        return QUEUE;
    }

    private static Queue<Job> initQueue() {
        Queue<Job> jobQueue = new LinkedList<Job>();

        return jobQueue;
    }

    private static class JobWatcher extends Thread {
        private boolean shutdown;

        @Override
        public void run() {
            while (!shutdown) {
                Queue<Job> queueRef = getQueue();
                // work as hard as possible (no sleep) while there are queued jobs
                while (!queueRef.isEmpty()) {
                    Job job = queueRef.iterator().next();
                    logger.debug(String.format("Processing job %s", job.toString()));
                    job.process();
                    queueRef.remove(job);
                    logger.debug(String.format("Finished processing job %s, removed from queue.", job.toString()));
                }

                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    // ignore
                }
            }
        }

        public void shutdown() {
            shutdown = true;
        }
    }
}
