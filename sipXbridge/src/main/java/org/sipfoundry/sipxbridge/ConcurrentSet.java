package org.sipfoundry.sipxbridge;

/**
 * This little utility is because java 5 does not support concurrent sets.
 * 
 */
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Semaphore;

class ConcurrentSet<T> implements Set<T> {

	
	ConcurrentHashMap<T,T> map = new ConcurrentHashMap<T,T> ();

	public boolean add(T element) {
		map.put(element,element);
		return true;
	}

	
	public boolean addAll(Collection<? extends T> collection) {
		for (T t: collection) {
			this.add(t);
		}
		return true;
	}

	public void clear() {
		map.clear();
	}

	public boolean contains(Object obj) {
		return map.containsKey(obj);

	}

	public boolean containsAll(Collection<?> collection) {
		Set<T> set = map.keySet();
		return set.containsAll(collection);
	}

	public boolean isEmpty() {
		return map.isEmpty();
	}

	public Iterator<T> iterator() {

		return map.keySet().iterator();
	}

	public boolean remove(Object obj) {
		this.map.remove(obj);
		return true;
	}

	public boolean removeAll(Collection<?> collection) {
		for ( Object obj : collection) {
			this.map.remove(obj);
		}
		return true;
	}

	public boolean retainAll(Collection<?> collection) {
		throw new UnsupportedOperationException ("Unsupported");
	}

	public int size() {
		return map.size();
	}

	public Object[] toArray() {

		return this.map.keySet().toArray();
	}

	public <T> T[] toArray(T[] array) {
		return this.map.keySet().toArray(array)	;
	}


}
