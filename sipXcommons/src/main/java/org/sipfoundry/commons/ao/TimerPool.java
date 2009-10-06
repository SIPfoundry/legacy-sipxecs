/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.ao;

import java.util.concurrent.*;
import java.util.concurrent.locks.*;

import static org.sipfoundry.commons.ao.ActiveObject.TimerState.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class TimerPool {
	private DelayQueue<ActiveObject.Timer> timerQueue;
	private TimerPoolThread timerPoolThread;

	public TimerPool(String name) {
		timerQueue = new DelayQueue<ActiveObject.Timer>();
		timerPoolThread = new TimerPoolThread(name + "-TimerPool");
		timerPoolThread.start();
	}

	public void add(ActiveObject.Timer timer) {
		timerQueue.add(timer);
	}

	public void remove(ActiveObject.Timer timer) {
		timerQueue.remove(timer);
	}

	/**
	 * Private implementation of the timer pool's Timer expiration thread which waits
	 * for timers on the delay queue, processing them at their scheduled expiration time.
	 * For timers which are configured to repeat, they are reloaded and placed back onto
	 * the delay queue.
	 */
	class TimerPoolThread extends Thread {
		public TimerPoolThread(String poolName) {
			super(poolName);
			this.setDaemon(true);
		}

		public void run() {
			try {
				mainLoop();
			} finally {
				// Someone killed this Thread, behave as if the Timer pool is being shut-down.
				synchronized (timerQueue) {
					timerQueue.clear(); // Eliminate obsolete references
				}
			}
		}

		/**
		 * The main Timer processing loop.
		 */
		private void mainLoop() {
			while (true) {
				try {
					ActiveObject.Timer timer;
					long currentTime;
					timer = timerQueue.take();
					currentTime = System.currentTimeMillis();
					final ReentrantLock lock = timer.lock;
					lock.lock();
					try {
						timer.expiredTime = currentTime;
						if (timer.period == 0) {
							// Non-repeating.
							timer.state = EXPIRED;
						} else {
							// Repeating Timer, reschedule.
							if (timer.fixedRate) {
								timer.expireTime = timer.expireTime + timer.period;
							} else {
								timer.expireTime = currentTime + timer.period;
							}
							timerQueue.put(timer);
						}
						// TODO: Need to put some logging here.
						timer.expire(currentTime);
					} finally {
						lock.unlock();
					}
				} catch (InterruptedException e) {
				}
			}
		}
	}
}
