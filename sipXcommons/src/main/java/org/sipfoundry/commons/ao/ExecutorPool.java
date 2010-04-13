/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.ao;

import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.locks.*;
import java.util.concurrent.atomic.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ExecutorPool {

	private final ReentrantLock lock = new ReentrantLock();

	private final AtomicInteger idleExecutorCount = new AtomicInteger(0);

	/**
	 * Set containing all worker threads in pool.
	 */
	private final HashSet<Executor> executorPool = new HashSet<Executor>();

	private final int maxExecutors;

	private final long maxExecutorIdleTime;

	/**
	 * Queue used for holding tasks and handing off to worker threads.
	 */
	private final BlockingQueue<AOInvocationHandler> taskQueue;

	private final ThreadGroup threadGroup;

	private final int executorThreadPriority;

	private final String poolName;

	private final AtomicInteger threadNumber = new AtomicInteger(1);

	/**
	 * Wait condition to support awaitTermination
	 */
	private final Condition termination = lock.newCondition();

	/**
	 * Lifecycle state
	 */
	volatile int runState;

	// Special values for runState
	/** Normal, not-shutdown mode */
	static final int RUNNING = 0;
	/** Controlled shutdown mode */
	static final int SHUTDOWN = 1;
	/** Immediate shutdown mode */
	static final int STOP = 2;
	/** Final state */
	static final int TERMINATED = 3;

	ExecutorPool(String poolName, int executorPriority, int maxExecutors, long maxExecutorIdleTime) {
		this.poolName = poolName;
		taskQueue = new LinkedBlockingQueue<AOInvocationHandler>();
		threadGroup = new ThreadGroup(poolName);
		executorThreadPriority = executorPriority;
		this.maxExecutors = maxExecutors;
		this.maxExecutorIdleTime = maxExecutorIdleTime;

	}

	public boolean awaitTermination(long timeout, TimeUnit unit) throws InterruptedException {
		long nanos = unit.toNanos(timeout);
		try {
			lock.lock();
			for (;;) {
				if (runState == TERMINATED)
					return true;
				if (nanos <= 0)
					return false;
				lock.unlock();
				nanos = termination.awaitNanos(nanos);
				lock.lock();
			}
		} finally {
			lock.unlock();
		}
	}

	public boolean isShutdown() {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean isTerminated() {
		// TODO Auto-generated method stub
		return false;
	}

	public void shutdown() {
		// TODO Auto-generated method stub

	}

	public List<Runnable> shutdownNow() {
		// TODO Auto-generated method stub
		return null;
	}

	/**
	 * Attempts to execute the given task by using the following strategy:
	 *
	 * If there is an idle executor thread in the pool, the task will be
	 * assigned to it. If not, a new executor thread will be allocated from the
	 * system. If the maximum allowable number of executor threads has already
	 * been allocated, then the task will be placed on the work queue. If the
	 * work queue is full, then the task will be rejected.
	 *
	 * @param task
	 *            the task to execute
	 * @throws RejectedExecutionException
	 *             at discretion of <tt>RejectedExecutionHandler</tt>, if
	 *             task cannot be accepted for execution
	 */
	public void execute(AOInvocationHandler task) {
		if (runState != RUNNING) {
			reject(task);
			return;
		}
		try {
			lock.lock();
			if (idleExecutorCount.get() > 0) {
				// Allocate task to an idle executor thread by placing on
				// what should be an empty task queue for which the idle
				// threads are blocking on.
				if (!taskQueue.offer(task)) {
					// There was an unexpected problem adding the task to the
					// queue.
					reject(task);
				} else {
					idleExecutorCount.decrementAndGet();
				}
			} else if (executorPool.size() < maxExecutors) {
				// Allocate a new executor thread and assign to the task.
				Executor executor = new Executor(task);
				String threadName = poolName;
				if (maxExecutors > 1) {
					threadName = threadName + "(" + threadNumber.getAndIncrement() + ")";
				}
				Thread thread = new Thread(threadGroup, executor, threadName, 0);
				thread.setDaemon(false);
				thread.setPriority(executorThreadPriority);
				executor.thread = thread;
				executorPool.add(executor);
				thread.start();
			} else if (taskQueue.offer(task)) {
				// Task placed on task queue for later servicing.
			} else {
				// The pool has reached maximum capacity, reject the task.
				reject(task);
			}
		} finally {
			lock.unlock();
		}

	}

	/**
	 * Perform bookkeeping for a terminated executor thread.
	 *
	 * @param w
	 *            the Executor
	 */
	void executorDone(Executor executor) {
		try {
			lock.lock();
			idleExecutorCount.decrementAndGet();
			executorPool.remove(executor);

			if (executorPool.size() > 0)
				return;

			// Else, this is the last thread. Deal with potential shutdown.

			int state = runState;

			if (state != STOP) {
				// If there are queued tasks but no threads, create
				// replacement thread. We must create it initially
				// idle to avoid orphaned tasks in case addThread
				// fails. This also handles case of delayed tasks
				// that will sometime later become runnable.
				if (!taskQueue.isEmpty()) {
					AOInvocationHandler task = taskQueue.poll();
					if (task != null) {
						// Allocate a new executor thread and assign to the task.
						Executor replacementExecutor = new Executor(task);
						String threadName = poolName;
						if (maxExecutors > 1) {
							threadName = threadName + "(" + threadNumber.getAndIncrement() + ")";
						}
						Thread thread = new Thread(threadGroup, replacementExecutor, threadName, 0);
						thread.setDaemon(false);
						thread.setPriority(executorThreadPriority);
						replacementExecutor.thread = thread;
						executorPool.add(replacementExecutor);
						thread.start();
					}
					return;
				}

				// Otherwise, we can exit without replacement
				if (state == RUNNING)
					return;
			}

			// Either state is STOP, or state is SHUTDOWN and there is
			// no work to do. So we can terminate.
			termination.signalAll();
			runState = TERMINATED;
		} finally {
			lock.unlock();
		}
	}

	private void reject(Runnable task) {

	}

	private Runnable nextTask() throws InterruptedException {
		Runnable task;
		try {
			lock.lock();
			if ((task = taskQueue.peek()) == null) {
				idleExecutorCount.incrementAndGet();
			} else {
				lock.unlock();
				task = taskQueue.take();
				lock.lock();
			}
		} finally {
			lock.unlock();
		}

		if (task == null) {
			task = taskQueue.poll(maxExecutorIdleTime, TimeUnit.MILLISECONDS);
		}

		return task;
	}

	/**
	 * Executor threads
	 */
	private class Executor implements Runnable {
		/**
		 * Initial task to run before entering run loop
		 */
		private AOInvocationHandler firstTask;

		/**
		 * Thread this worker is running in. Acts as a final field, but cannot
		 * be set until thread is created.
		 */
		Thread thread;

		Executor(AOInvocationHandler firstTask) {
			this.firstTask = firstTask;
		}

		/**
		 * Main run loop
		 */
		public void run() {
			try {
				Runnable task = firstTask;
				firstTask = null; // Remove the reference.
				while (task != null) {
					task.run();
					task = null; // Free up the reference immediately.
					task = nextTask();
				}
			} catch (InterruptedException ie) {
				// fall through
			} finally {
				executorDone(this);
			}
		}
	}

}
