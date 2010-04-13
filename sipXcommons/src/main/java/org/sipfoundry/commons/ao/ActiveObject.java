/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.ao;

import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.locks.*;
import java.lang.reflect.*;

import org.apache.log4j.Logger;

import static org.sipfoundry.commons.ao.ActiveObject.TimerState.*;
import static org.sipfoundry.commons.ao.MethodAttribute.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ActiveObject {
	/**
	 * Enumerations for timer state.
	 */
	public enum TimerState {
		/**
		 * This timer has not yet been scheduled.
		 */
		IDLE,

		/**
		 * This timer is scheduled and has not yet expired.
		 */
		SCHEDULED,

		/**
		 * This timer has already expired.
		 */
		EXPIRED,

		/**
		 * This timer has been canceled.
		 */
		CANCELLED;
	}

	protected static HashMap<Class<? extends ActiveObject>, HashMap<Method, MethodAttribute>> methodAttributesCache = null;
	protected static HashMap<Class<? extends ActiveObject>, HashMap<String, TimerTickMethod>> timerTickMethodsCache = null;
	protected static HashMap<Class<? extends ActiveObject>, Method> startupMethods = null;
	protected static HashMap<Class<? extends ActiveObject>, Method> shutdownMethods = null;

	String label;
	private Logger logger;
	protected TimerPool timerPool;
	protected HashMap<String, TimerTickMethod> timerTickMethods;
	protected Object thisActiveObject;
	protected Object groupMemberID;
	//protected ActiveObjectGroup<? extends ActiveObjectGroup> activeObjectGroup;
	protected ActiveObjectGroupInterface activeObjectGroup;

	public ActiveObject() {
		this("ActiveObject");
	}

	public ActiveObject(String label) {
		this.label = label;
	}

	protected static Object newActiveObject(ActiveObject object, ExecutorPool executorPool, TimerPool timerPool, Logger logger) {
		HashMap<Method, MethodAttribute> methodAttributes;
		HashMap<String, TimerTickMethod> timerTickMethods;
		if (methodAttributesCache == null) {
			methodAttributesCache = new HashMap<Class<? extends ActiveObject>, HashMap<Method, MethodAttribute>>();
			timerTickMethodsCache = new HashMap<Class<? extends ActiveObject>, HashMap<String, TimerTickMethod>>();
			startupMethods = new HashMap<Class<? extends ActiveObject>, Method>();
			shutdownMethods = new HashMap<Class<? extends ActiveObject>, Method>();
		}
		methodAttributes = methodAttributesCache.get(object.getClass());
		if (methodAttributes == null) {
			timerTickMethods = new HashMap<String, TimerTickMethod>();
			timerTickMethodsCache.put(object.getClass(), timerTickMethods);
			methodAttributes = new HashMap<Method, MethodAttribute>();
			getMethodAttributes(object, methodAttributes, timerTickMethods, startupMethods, shutdownMethods);
			methodAttributesCache.put(object.getClass(), methodAttributes);
		} else {
			timerTickMethods = timerTickMethodsCache.get(object.getClass());
		}

		Object activeObject = Proxy.newProxyInstance(object.getClass().getClassLoader(),
				object.getClass().getInterfaces(),
				new AOInvocationHandler(object, executorPool, methodAttributes, logger));

		object.timerPool = timerPool;
		object.timerTickMethods = timerTickMethods;
		object.thisActiveObject = activeObject;
		object.groupMemberID = null;
		object.activeObjectGroup = null;

		Method startupMethod;
		if ((startupMethod = startupMethods.get(object.getClass())) != null) {
			Object[] null_args = {};
			try {
				startupMethod.invoke(activeObject, null_args);
			} catch (IllegalArgumentException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (InvocationTargetException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		return activeObject;
	}

	public static Object newActiveObject(ActiveObject object) {
		return newActiveObject(object, new ExecutorPool(object.label, Thread.NORM_PRIORITY, 1,
				60000L), new TimerPool(object.label + "-TimerPool"), object.logger);
	}

	public void setThisActiveObject(Object proxy) {
		this.thisActiveObject = proxy;
	}

	public void setID(Object id) {
		this.groupMemberID = id;
	}

	public Object getID() {
		return groupMemberID;
	}

	public void setActiveObjectGroup(ActiveObjectGroupInterface activeObjectGroup) {
		this.activeObjectGroup = activeObjectGroup;
	}

	public Object getThisActiveObject() {
	    return thisActiveObject;
	}
	
	public ActiveObjectGroupInterface getActiveObjectGroup() {
		return activeObjectGroup;
	}

	private static void getMethodAttributes(ActiveObject object,
			HashMap<Method, MethodAttribute> methodAttributes,
			HashMap<String, TimerTickMethod> timerTickMethods,
			HashMap<Class<? extends ActiveObject>, Method> startupMethods,
			HashMap<Class<? extends ActiveObject>, Method> shutdownMethods) {
		Class<? extends Object> thisClass = object.getClass();
		Method[] implementationMethodList = thisClass.getMethods();
		Class<?>[] interfaces = thisClass.getInterfaces();
		for (int x = 0; x < interfaces.length; x++) {
			Method[] interfaceMethodList = interfaces[x].getMethods();
			for (int y = 0; y < interfaceMethodList.length; y++) {
				Method interfaceMethod = interfaceMethodList[y];
				Class<?>[] interfaceMethodParams = interfaceMethod.getParameterTypes();
				for (int z = 0; z < implementationMethodList.length; z++) {
					Method implementationMethod = implementationMethodList[z];
					if (interfaceMethod.getName().compareTo(implementationMethod.getName()) == 0) {
						Class<?>[] implementationMethodParams = implementationMethod.getParameterTypes();
						if (interfaceMethodParams.length == implementationMethodParams.length) {
							boolean foundMatch = true;
							for (int q = 0; q < interfaceMethodParams.length; q++) {
								if (!interfaceMethodParams[q].equals(implementationMethodParams[q])) {
									foundMatch = false;
									break;
								}
							}
							if (foundMatch) {
								MethodAttribute methodAttribute = new MethodAttribute();
								if (implementationMethod.isAnnotationPresent(HighPriority.class)) {
									methodAttribute.priority = HIGH_PRIORITY;
								} else if (implementationMethod.isAnnotationPresent(LowPriority.class)) {
									methodAttribute.priority = LOW_PRIORITY;
								} else {
									methodAttribute.priority = NORMAL_PRIORITY;
								}

								if (implementationMethod.isAnnotationPresent(Blocking.class)) {
									methodAttribute.blocking = true;
								} else {
									methodAttribute.blocking = false;
								}

								if (implementationMethod.isAnnotationPresent(Synchronous.class)) {
									methodAttribute.synchronous = true;
								} else {
									methodAttribute.synchronous = false;
								}

								if (implementationMethod.isAnnotationPresent(Shutdown.class)) {
									methodAttribute.terminate = true;
								} else {
									methodAttribute.terminate = false;
								}

								methodAttributes.put(interfaceMethod, methodAttribute);

								if (implementationMethod.isAnnotationPresent(TimerTick.class)) {
									TimerTick timerTickAnnotation = implementationMethod.getAnnotation(TimerTick.class);
									String tickMethod = timerTickAnnotation.value();
									if (implementationMethodParams.length == 0) {
										TimerTickMethod timerTickMethod = new TimerTickMethod();
										timerTickMethod.includeExpiredTime = false;
										timerTickMethod.method = interfaceMethod;
										timerTickMethods.put(tickMethod, timerTickMethod);
									} else if (implementationMethodParams.length == 1 && implementationMethodParams[0] == long.class) {
										TimerTickMethod timerTickMethod = new TimerTickMethod();
										timerTickMethod.includeExpiredTime = true;
										timerTickMethod.method = interfaceMethod;
										timerTickMethods.put(tickMethod, timerTickMethod);
									} else {
										System.err.println("TimerTick method can only accept single long argument.");
									}
								}

								if (implementationMethod.isAnnotationPresent(Startup.class)) {
									if (implementationMethodParams.length == 0) {
										startupMethods.put(object.getClass(), interfaceMethod);
									} else {
										System.err.println("Startup method cannot accept arguments.");
									}
								}

								if (implementationMethod.isAnnotationPresent(Shutdown.class)) {
									if (implementationMethodParams.length == 0) {
										shutdownMethods.put(object.getClass(), interfaceMethod);
									} else {
										System.err.println("Shutdown method cannot accept arguments.");
									}
								}
							}
						}
					}
				}
			}
		}
	}

	public class Timer implements Delayed {
		protected transient final ReentrantLock lock = new ReentrantLock();

		String label;

		/**
		 * The current state of this timer.
		 */
		protected TimerState state;

		protected TimerTickMethod tickMethod;

		private long delay;

		protected long expireTime;

		protected long period;

		protected boolean fixedRate;

		protected long expiredTime;


		/**
		 * Default constructor.
		 */
		public Timer() {
			this.label = null;
		}

		/**
		 * Default constructor.
		 */
		public Timer(String label) {
			this.label = label;
		}

		public void schedule(String tickMethodName, long delay) {
			final ReentrantLock lock = this.lock;
			if (delay < 0) {
				throw new IllegalArgumentException("Negative delay.");
			}
			if (state == SCHEDULED) {
				throw new IllegalStateException("Task already scheduled.");
			}
			if ((tickMethod = timerTickMethods.get(tickMethodName)) == null) {
				throw new IllegalArgumentException("Invalid tick method: " + tickMethodName);
			}

			lock.lock();
			try {
				this.delay = delay;
				expireTime = System.currentTimeMillis() + delay;
				this.period = 0;
				fixedRate = false;
				state = SCHEDULED;
				timerPool.add(this);
			} finally {
				lock.unlock();
			}
		}

		public void schedule(String tickMethodName, long delay, long period) {
			final ReentrantLock lock = this.lock;
			if (state == SCHEDULED) {
				throw new IllegalStateException("Task already scheduled.");
			}
			if (delay < 0) {
				throw new IllegalArgumentException("Negative delay.");
			}
			if (period <= 0) {
				throw new IllegalArgumentException("Non-positive period.");
			}

			if ((tickMethod = timerTickMethods.get(tickMethodName)) == null) {
				throw new IllegalArgumentException("Invalid tick method: " + tickMethodName);
			}

			lock.lock();
			try {
				this.delay = delay;
				expireTime = System.currentTimeMillis() + delay;
				this.period = period;
				fixedRate = false;
				state = SCHEDULED;
				timerPool.add(this);
			} finally {
				lock.unlock();
			}
		}

		public void scheduleAtFixedRate(String tickMethodName, long delay, long period) {
			final ReentrantLock lock = this.lock;
			if (state == SCHEDULED) {
				throw new IllegalStateException("Task already scheduled.");
			}
			if (delay < 0) {
				throw new IllegalArgumentException("Negative delay.");
			}
			if (period <= 0) {
				throw new IllegalArgumentException("Non-positive period.");
			}
			if ((tickMethod = timerTickMethods.get(tickMethodName)) == null) {
				throw new IllegalArgumentException("Invalid tick method: " + tickMethodName);
			}

			lock.lock();
			try {
				this.delay = delay;
				expireTime = System.currentTimeMillis() + delay;
				this.period = period;
				fixedRate = true;
				state = SCHEDULED;
				timerPool.add(this);
			} finally {
				lock.unlock();
			}
		}

		public void cancel() {
			final ReentrantLock lock = this.lock;
			lock.lock();
			try {
				if (state == SCHEDULED) {
					timerPool.remove(this);
				}
				state = CANCELLED;
			} finally {
				lock.unlock();
			}
		}

		public long getExpiredTime() {
			final ReentrantLock lock = this.lock;
			lock.lock();
			try {
				if (state == CANCELLED) {
					return 0;
				} else {
					return expiredTime;
				}
			} finally {
				lock.unlock();
			}
		}

		public void reset() {
			final ReentrantLock lock = this.lock;
			lock.lock();
			try {
				if (state == SCHEDULED) {
					timerPool.remove(this);
				}
				expireTime = System.currentTimeMillis() + delay;
				state = SCHEDULED;
				timerPool.add(this);
			} finally {
				lock.unlock();
			}
		}

		protected void expire(long expiredTime) {
			try {
				Object[] null_args = {};
				Object[] args = { expiredTime };
				if (tickMethod.includeExpiredTime) {
					tickMethod.method.invoke(thisActiveObject, args);
				} else {
					tickMethod.method.invoke(thisActiveObject, null_args);
				}
			} catch (IllegalArgumentException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (InvocationTargetException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		/**
		 * Returns the remaining delay associated with this timer, in the given
		 * time unit.
		 *
		 * @param unit
		 *            the time unit
		 * @return the remaining delay; zero or negative values indicate that
		 *         the delay has already elapsed
		 */
		public long getDelay(TimeUnit unit) {
			return unit.convert(expireTime - System.currentTimeMillis(), TimeUnit.MILLISECONDS);
		}

		/**
		 * Compares this timer with the specified timer for order. Returns a
		 * negative integer, zero, or a positive integer as this timers delay is
		 * less than, equal to, or greater than the specified object.
		 * <p>
		 *
		 * @param timer
		 *            the AOTimer to be compared.
		 * @return a negative integer, zero, or a positive integer as this
		 *         timers delay is less than, equal to, or greater than the
		 *         specified timer.
		 */
		public int compareTo(Delayed timer) {
			long thisDelay = expireTime - System.currentTimeMillis();
			long comparingDelay = timer.getDelay(TimeUnit.MILLISECONDS);
			if (thisDelay < comparingDelay) {
				return -1;
			} else if (thisDelay == comparingDelay) {
				return 0;
			} else {
				return 1;
			}
		}

	}

}
