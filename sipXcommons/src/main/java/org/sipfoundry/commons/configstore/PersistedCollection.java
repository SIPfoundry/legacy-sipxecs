package org.sipfoundry.commons.configstore;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Date;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Set;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class PersistedCollection<V extends PersistedClass> implements Iterable<V> {
    private final String DbTable;
    private final Class<V> persistedClass;
    private String primaryDbKey;

    /**
     * Mapping between persisted objects and keys.
     */
    private final ConcurrentHashMap<String, V> persistedObjectMap;

    /**
     * Mapping between persisted fields and DB columns.
     */
    private HashMap<String, Field> persistedFieldsDbMap;

    /**
     * Mapping between persisted fields and their names.
     */
    private HashMap<String, Field> persistedFieldsMap;

    /**
     * Mapping between persisted fields and update notifier methods.
     */
    private HashMap<Field, Method> updateNotifiersMap;

    public PersistedCollection(String DbTable, Class<V> persistedClass) {
        this.DbTable = DbTable;
        this.persistedClass = persistedClass;

        // Instantiate the persisted object map.
        persistedObjectMap = new ConcurrentHashMap<String, V>();

        // Build up the persistedFieldsMap.
        buildPersistedFieldsMap();

        // Build up the updateNotifiersMap.
        buildUpdateNotifiersMap();

    }

    /**
     * Attempt to find the persisted object instance that has previously been mapped to a specific key.
     * 
     * @param key
     *            The key that has previously been mapped to a persisted object instance.
     * @return The persisted object instance that the given key maps to. If no mapping exists, returns null.
     */
    public synchronized V get(String key) {
        if (key != null) {
            return persistedObjectMap.get(key);
        } else {
            return null;
        }
    }

    public synchronized int size() {
        return persistedObjectMap.size();
    }

    public synchronized boolean isEmpty() {
        return persistedObjectMap.isEmpty();
    }

    public Iterator<V> iterator() {
        return new PersistedCollectionIterator();
    }

    public void entryCreated(V entry) {
    }

    public void entryModified(V entry) {
    }

    public void entryDeleted(V entry) {
    }

    protected String getDbTable() {
        return DbTable;
    }

    protected String getPrimaryDbKey() {
        return primaryDbKey;
    }

    protected synchronized void processRowUpdate(ResultSet rs, ConnectionListenerInterface connectionListener) throws SQLException {
        // Convert the ResultSet into a list for easy parsing.
        LinkedList<String> rowList = new LinkedList<String>();
        String rowKey;
        while (rs.next()) {
            rowKey = rs.getString(1).trim();
            rowList.add(rowKey);
        }

        // Now compare the list of DB rows with the collection to see if any have been deleted.
        Enumeration<V> entries = persistedObjectMap.elements();
        while (entries.hasMoreElements()) {
            V entry = entries.nextElement();
            if (!rowList.contains(entry.getKey())) {
                // This entry was deleted from the database.
                // First call the optional entryDeleted() callback.
                entryDeleted(entry);

                // Now remove from list.
                if (persistedObjectMap.containsKey(entry.getKey())) {
                    persistedObjectMap.remove(entry.getKey());
                }
            }
        }

        // Finally compare the current collection with the DB rows to see if any have been added.
        for (String row : rowList) {
            if (!persistedObjectMap.containsKey(row)) {
                // New entry found in the database.
                // Create a new corresponding instance.
                V entry = newInstance();
                entry.setKey(row);

                // Set the instance fields.
                ResultSet rowResults = connectionListener.retrieveRow(DbTable, primaryDbKey, row);
                rowResults.next();
                try {
                    Set<Entry<String, Field>> fieldSet = persistedFieldsDbMap.entrySet();
                    for (Entry<String, Field> fieldEntry : fieldSet) {
                        Field field = fieldEntry.getValue();
                        String dbColumn = fieldEntry.getKey();
                        // Temporarily override field access control.
                        field.setAccessible(true);
                        if (dbColumn.compareToIgnoreCase(primaryDbKey) == 0) {
                            // Special case for table key.
                            synchronized (entry) {
                                field.set(entry, row.trim());
                            }
                        } else if (field.getType() == String.class) {
                            String value = rowResults.getString(dbColumn);
                            synchronized (entry) {
                                if (value != null) {
                                    field.set(entry, value.trim());
                                } else {
                                    field.set(entry, null);
                                }
                            }
                        } else if (field.getType() == int.class) {
                            int value = rowResults.getInt(dbColumn);
                            synchronized (entry) {
                                field.setInt(entry, value);
                            }
                        } else if (field.getType() == Float.class) {
                            Float value = rowResults.getFloat(dbColumn);
                            synchronized (entry) {
                                field.setFloat(entry, value);
                            }
                        } else if (field.getType() == boolean.class) {
                            int value = rowResults.getInt(dbColumn);
                            synchronized (entry) {
                                field.setBoolean(entry, (value == 0 ? false : true));
                            }
                        } else if (field.getType() == Date.class) {
                            Date value = rowResults.getDate(dbColumn);
                            synchronized (entry) {
                                field.set(entry, value);
                            }
                        } else {
                            // Unsupported field type encountered.
                        }
                        field.setAccessible(false);
                    }
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                }

                rowResults.close();

                // Add to the persistedObjectMap.
                persistedObjectMap.put(row, entry);

                // Finally call the optional entryCreated() callback.
                entryCreated(entry);
            }
        }
    }

    protected synchronized void processColumnUpdate(ResultSet rs, ConnectionListenerInterface connectionListener) throws SQLException {
        while (rs.next()) {
            boolean entryUpdated = false;
            V entry = persistedObjectMap.get(rs.getString(primaryDbKey).trim());
            try {
                Set<Entry<String, Field>> fieldSet = persistedFieldsDbMap.entrySet();
                for (Entry<String, Field> fieldEntry : fieldSet) {
                    Field field = fieldEntry.getValue();
                    String dbColumn = fieldEntry.getKey();
                    // Temporarily override field access control.
                    field.setAccessible(true);
                    if (dbColumn.compareToIgnoreCase(primaryDbKey) == 0) {
                        // Skip table key.
                        continue;
                    } else if (field.getType() == String.class) {
                        String value = rs.getString(dbColumn);
                        if (value != null) {
                            value = value.trim();
                        }
                        String origValue = (String) field.get(entry);
                        if ((origValue == null || value == null) && origValue != value) {
                            synchronized (entry) {
                                field.set(entry, value);
                            }
                            fieldUpdated(entry, field);
                            entryUpdated = true;
                        } else if (origValue != null && value != null && origValue.compareTo(value) != 0) {
                            synchronized (entry) {
                                field.set(entry, value);
                            }
                            fieldUpdated(entry, field);
                            entryUpdated = true;
                        }
                    } else if (field.getType() == int.class) {
                        int value = rs.getInt(dbColumn);
                        if (value != field.getInt(entry)) {
                            synchronized (entry) {
                                field.setInt(entry, value);
                            }
                            fieldUpdated(entry, field);
                            entryUpdated = true;
                        }
                    } else if (field.getType() == Float.class) {
                        Float value = rs.getFloat(dbColumn);
                        if (value != field.getFloat(entry)) {
                            synchronized (entry) {
                                field.setFloat(entry, value);
                            }
                            fieldUpdated(entry, field);
                            entryUpdated = true;
                        }
                    } else if (field.getType() == boolean.class) {
                        boolean value = (rs.getInt(dbColumn) == 0 ? false : true);
                        if (value != field.getBoolean(entry)) {
                            synchronized (entry) {
                                field.setBoolean(entry, value);
                            }
                            fieldUpdated(entry, field);
                            entryUpdated = true;
                        }
                    } else if (field.getType() == Date.class) {
                        Date origValue = (Date) field.get(entry);
                        Date value = rs.getDate(dbColumn);
                        if (origValue.compareTo(value) != 0) {
                            synchronized (entry) {
                                field.set(entry, value);
                            }
                            fieldUpdated(entry, field);
                            entryUpdated = true;
                        }
                    } else {
                        // Unsupported field type encountered.
                    }
                    field.setAccessible(false);
                }
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
            if (entryUpdated) {
                entryModified(entry);
                entryUpdated = false;
            }
        }
    }

    private V newInstance() {
        V newObj = null;

        try {
            newObj = persistedClass.newInstance();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }

        return newObj;
    }

    private void buildPersistedFieldsMap() {
        persistedFieldsMap = new HashMap<String, Field>();
        persistedFieldsDbMap = new HashMap<String, Field>();
        // Walk the inheritance chain and build up a list of classes.
        LinkedList<Class<?>> clazzList = new LinkedList<Class<?>>();
        Class<?> clazz = persistedClass;
        while (clazz != PersistedClass.class) {
            clazzList.add(clazz);
            clazz = clazz.getSuperclass();
        }
        // Walk through the list of classes looking for PersistedField annotations.
        for (Class<?> clazzz : clazzList) {
            Field[] fields = clazzz.getDeclaredFields();
            for (Field field : fields) {
                if (field.isAnnotationPresent(PersistedField.class)) {
                    PersistedField annotation = field.getAnnotation(PersistedField.class);
                    String dbColumn = annotation.value().toLowerCase();
                    if (dbColumn.compareTo("") == 0) {
                        dbColumn = field.getName().toLowerCase();
                    }
                    persistedFieldsMap.put(field.getName(), field);
                    persistedFieldsDbMap.put(dbColumn, field);

                    if (field.isAnnotationPresent(PrimaryKey.class)) {
                        primaryDbKey = dbColumn;
                    }
                }
            }
        }
    }

    private void buildUpdateNotifiersMap() {
        updateNotifiersMap = new HashMap<Field, Method>();
        // Walk the inheritance chain and build up a list of classes.
        LinkedList<Class<?>> clazzList = new LinkedList<Class<?>>();
        Class<?> clazz = persistedClass;
        while (clazz != PersistedClass.class) {
            clazzList.add(clazz);
            clazz = clazz.getSuperclass();
        }
        // Walk through the list of classes looking for PersistedField annotations.
        for (Class<?> clazzz : clazzList) {
            Method[] methods = clazzz.getDeclaredMethods();
            for (Method method : methods) {
                if (method.isAnnotationPresent(UpdateNotifier.class)) {
                    UpdateNotifier annotation = method.getAnnotation(UpdateNotifier.class);
                    String[] fieldList = annotation.value();
                    for (String fieldName : fieldList) {
                        Field field = persistedFieldsMap.get(fieldName);
                        if (field != null) {
                            updateNotifiersMap.put(field, method);
                        } else {
                            // Could not find matching field.
                            System.out.println("Could not find matching field for: " + fieldName);
                        }
                    }

                }
            }
        }
    }

    private void fieldUpdated(V element, Field field) {
        Method updateNotifierMethod = updateNotifiersMap.get(field);
        if (updateNotifierMethod != null) {
            try {
                updateNotifierMethod.invoke(element, field);
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            }
        }
    }

    public class PersistedCollectionIterator implements Iterator<V> {
        private final Object[] collection;
        private int index;

        public PersistedCollectionIterator() {
            collection = persistedObjectMap.values().toArray();
            index = 0;
        }

        public boolean hasNext() {
            if (collection.length > index) {
                return true;
            } else {
                return false;
            }
        }

        @SuppressWarnings("unchecked")
        public V next() {
            if (collection.length > index) {
                return (V) collection[index++];
            } else {
                return null;
            }
        }

        public void remove() {
            throw new UnsupportedOperationException();
        }
    }
}
