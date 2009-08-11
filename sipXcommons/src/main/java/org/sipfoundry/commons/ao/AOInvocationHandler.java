/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.ao;

import static org.sipfoundry.commons.ao.MethodAttribute.*;

import java.lang.reflect.*;
import java.util.HashMap;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.locks.ReentrantLock;
import org.apache.log4j.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class AOInvocationHandler implements InvocationHandler, Runnable {
	/**
	 * Enumerations for representing the state of the ActiveObject.
	 */
	public enum State {
		/**
		 * The ActiveObject does not have a context.
		 */
		IDLE,

		/**
		 * The ActiveObject context is running.
		 */
		ACTIVE,

		/**
		 * The ActiveObject has been shut-down.
		 */
		TERMINATED,

		/**
		 * The ActiveObject has encountered a runtime exception.
		 */
		EXCEPTION;

	}

	private class Invocation {
		protected Method method;
		protected Object[] args;
		protected boolean highPriority;
		protected boolean blocking;
		protected boolean terminate;
		protected SynchronousQueue<Object> results = null;

	}

	/** Main lock guarding all access */
	private final ReentrantLock lock;

	/**
	 * The current state of the ActiveObject.
	 */
	private volatile State state = State.IDLE;

	/**
	 * The invocation request queue used for queuing proxied Object method
	 * invocations.
	 */
	private final InvocationQueue invocationQueue;

	private final Object object;

	private Thread executor;

	private final ExecutorPool executorPool;

	private final HashMap<Method, MethodAttribute> methodCache;

	@SuppressWarnings("unused")
    private final Logger logger;

	protected AOInvocationHandler(Object object, ExecutorPool executorPool, HashMap<Method, MethodAttribute> methodCache, Logger logger) {
		this.object = object;
		this.executorPool = executorPool;
		this.methodCache = methodCache;
		this.logger = logger;
		lock = new ReentrantLock(true);
		invocationQueue = new InvocationQueue(32);

	}

	public synchronized void terminate() {
		// if (logger.getLevel() == Level.DEBUG)
		// logger.debug("ActiveObject: " + groupName + "(" + id + ") terminated");

		// If there is an associated executor, jolt it.
		if (state == State.ACTIVE) {
			state = State.TERMINATED;
			executor.interrupt();
		}

		state = State.TERMINATED;

	}

	public synchronized Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
		Invocation invocation;
		int priority;
		Object results = null;
		final ReentrantLock lock = this.lock;

		MethodAttribute AOMethod = methodCache.get(method);
		if (AOMethod == null) {
			state = State.EXCEPTION;
			System.err.println("Failed to find method: " + method.getName() + " in cache.");
			return results;
		}

		lock.lock();
		try {
			invocation = new Invocation();
			invocation.method = method;
			invocation.args = args;

			if (AOMethod.synchronous) {
				try {
					results = method.invoke(object, args);
				} catch (Throwable t) {
					state = State.EXCEPTION;
					System.err.println("Synchronous Invocation Exception!");
				}

				if (AOMethod.terminate) {
				    terminate();
				}
			} else {
				priority = AOMethod.priority;

				if (AOMethod.blocking) {
					invocation.blocking = true;
					invocation.results = new SynchronousQueue<Object>();
				} else {
				    invocation.blocking = false;
				}

				if (AOMethod.blocking) {
				    invocation.terminate = true;
				} else {
				    invocation.terminate = false;
				}

				if (invocationQueue.push(invocation, priority) == false) {
					System.err.println("PUSH Failed.");
				}

				// If there is currently no thread assigned, re-execute.
				if (state == State.IDLE) {
					// Ask the ExecutorPool to assign a thread.
					state = State.ACTIVE;
					executorPool.execute(this);
				}
			}
		} finally {
			lock.unlock();
		}

		if (AOMethod.blocking) {
			results = invocation.results.take();
		}

		return results;
	}

	public void run() {
		Invocation invocation;
		Object results;

		// Update the executor thread reference.
		synchronized (this) {
			executor = Thread.currentThread();
		}

		while (state == State.ACTIVE) {
			final ReentrantLock lock = this.lock;
			lock.lock();
			try {
				if ((invocation = invocationQueue.pop()) == null) {
					// The FIFO has been drained, update the state and release the executor.
					executor = null;
					state = State.IDLE;
					continue;
				}
			} finally {
				lock.unlock();
			}
			try {
				results = invocation.method.invoke(object, invocation.args);
				if (invocation.blocking) {
					if (results != null) {
						invocation.results.put(results);
					} else {
						invocation.results.put(0);
					}
				}

				if (invocation.terminate) {
				    terminate();
				}
			} catch (Exception e) {
				// Shutting down.
				state = State.EXCEPTION;
				e.printStackTrace();
				continue;
			}
		}

	}

	public State getState() {
		return state;
	}

	private class InvocationQueue {
		private final int capacity;

		Invocation[] low_invocations;
		transient int low_count;
		transient int low_pushIndex;
		transient int low_popIndex;

		Invocation[] normal_invocations;
		transient int normal_count;
		transient int normal_pushIndex;
		transient int normal_popIndex;

		Invocation[] high_invocations;
		transient int high_count;
		transient int high_pushIndex;
		transient int high_popIndex;

		/**
		 * Creates a nonblocking FIFO queue with the given (fixed) capacity.
		 *
		 * @param capacity
		 *            the capacity of this queue
		 */
		public InvocationQueue(int capacity) {
			this.capacity = capacity;

			low_invocations = new Invocation[capacity];
			low_count = 0;
			low_pushIndex = 0;
			low_popIndex = 0;

			normal_invocations = new Invocation[capacity];
			normal_count = 0;
			normal_pushIndex = 0;
			normal_popIndex = 0;

			high_invocations = new Invocation[capacity];
			high_count = 0;
			high_pushIndex = 0;
			high_popIndex = 0;

		}

		/**
		 * Inserts the specified element at the tail of this queue if possible,
		 * returning immediately if this queue is full.
		 *
		 * @param invocation
		 *            The element to add.
		 * @param priority
		 *            The priority of the element.
		 * @return <tt>true</tt> if it was possible to add the element to this
		 *         queue, <tt>false</tt> otherwise.
		 */
		public boolean push(Invocation invocation, int priority) {
			if (priority == HIGH_PRIORITY) {
				if (high_count == capacity)
					return false;
				else {
					high_invocations[high_pushIndex] = invocation;
					high_pushIndex = ((++high_pushIndex == high_invocations.length) ? 0 : high_pushIndex);
					high_count++;
					return true;
				}
			} else if (priority == NORMAL_PRIORITY) {
				if (normal_count == capacity)
					return false;
				else {
					normal_invocations[normal_pushIndex] = invocation;
					normal_pushIndex = ((++normal_pushIndex == normal_invocations.length) ? 0 : normal_pushIndex);
					normal_count++;
					return true;
				}
			} else {
				if (low_count == capacity)
					return false;
				else {
					low_invocations[low_pushIndex] = invocation;
					low_pushIndex = ((++low_pushIndex == low_invocations.length) ? 0 : low_pushIndex);
					low_count++;
					return true;
				}
			}
		}

		public Invocation pop() {
			if (high_count != 0) {
				Invocation invocation = high_invocations[high_popIndex];
				high_invocations[high_popIndex] = null;
				high_popIndex = ((++high_popIndex == high_invocations.length) ? 0 : high_popIndex);
				high_count--;
				return invocation;
			} else if (normal_count != 0) {
				Invocation invocation = normal_invocations[normal_popIndex];
				normal_invocations[normal_popIndex] = null;
				normal_popIndex = ((++normal_popIndex == normal_invocations.length) ? 0 : normal_popIndex);
				normal_count--;
				return invocation;
			} else if (low_count != 0) {
				Invocation invocation = low_invocations[low_popIndex];
				low_invocations[low_popIndex] = null;
				low_popIndex = ((++low_popIndex == low_invocations.length) ? 0 : low_popIndex);
				low_count--;
				return invocation;
			} else {
				return null;
			}

		}

		/**
		 * Atomically removes all of the elements from this queue. The queue will be
		 * empty after this call returns.
		 */
		public void clear() {
			int x = high_popIndex;
			int count = high_count;
			high_count = 0;
			high_pushIndex = 0;
			high_popIndex = 0;
			while (count-- > 0) {
				high_invocations[x] = null;
				x = ((++x == high_invocations.length) ? 0 : x);
			}

			x = normal_popIndex;
			count = normal_count;
			normal_count = 0;
			normal_pushIndex = 0;
			normal_popIndex = 0;
			while (count-- > 0) {
				normal_invocations[x] = null;
				x = ((++x == normal_invocations.length) ? 0 : x);
			}

			x = low_popIndex;
			count = low_count;
			low_count = 0;
			low_pushIndex = 0;
			low_popIndex = 0;
			while (count-- > 0) {
				low_invocations[x] = null;
				x = ((++x == low_invocations.length) ? 0 : x);
			}
		}

	}

}
