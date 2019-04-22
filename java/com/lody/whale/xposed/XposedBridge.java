package com.lody.whale.xposed;

import android.util.Log;

import com.lody.whale.WhaleRuntime;
import com.lody.whale.xposed.XC_MethodHook.MethodHookParam;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Member;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

/**
 * This class contains most of Xposed's central logic, such as initialization and callbacks used by
 * the native side. It also includes methods to add new hooks.
 * <p>
 * Latest Update 2018/04/20
 */
@SuppressWarnings("WeakerAccess")
public final class XposedBridge {
    /**
     * The system class loader which can be used to locate Android framework classes.
     * Application classes cannot be retrieved from it.
     *
     * @see ClassLoader#getSystemClassLoader
     */
    @SuppressWarnings("unused")
    public static final ClassLoader BOOTCLASSLOADER = ClassLoader.getSystemClassLoader();

    public static final String TAG = "Whale-Buildin-Xposed";

    /*package*/ static boolean disableHooks = false;

    private static final Object[] EMPTY_ARRAY = new Object[0];

    // built-in handlers
    private static final Map<Member, CopyOnWriteSortedSet<XC_MethodHook>> sHookedMethodCallbacks = new HashMap<>();

    private static final Map<Member, Long> sHookedMethodSlotMap = new HashMap<>();

    /**
     * Writes a message to the logcat error log.
     *
     * @param text The log message.
     */
    @SuppressWarnings("unused")
    public static void log(final String text) {
        Log.i(TAG, text);
    }

    /**
     * Logs a stack trace to the logcat error log.
     *
     * @param t The Throwable object for the stack trace.
     */
    public static void log(final Throwable t) {
        Log.e(TAG, Log.getStackTraceString(t));
    }

    /**
     * Hook any method (or constructor) with the specified callback. See below for some wrappers
     * that make it easier to find a method/constructor in one step.
     *
     * @param hookMethod The method to be hooked.
     * @param callback   The callback to be executed when the hooked method is called.
     * @return An object that can be used to remove the hook.
     * @see XposedHelpers#findAndHookMethod(String, ClassLoader, String, Object...)
     * @see XposedHelpers#findAndHookMethod(Class, String, Object...)
     * @see #hookAllMethods
     * @see XposedHelpers#findAndHookConstructor(String, ClassLoader, Object...)
     * @see XposedHelpers#findAndHookConstructor(Class, Object...)
     * @see #hookAllConstructors
     */
    public static XC_MethodHook.Unhook hookMethod(final Member hookMethod, final XC_MethodHook callback) {
        if (!(hookMethod instanceof Method) && !(hookMethod instanceof Constructor<?>)) {
            throw new IllegalArgumentException("Only methods and constructors can be hooked: " + hookMethod.toString());
        } else if (hookMethod.getDeclaringClass().isInterface()) {
            throw new IllegalArgumentException("Cannot hook interfaces: " + hookMethod.toString());
        } else if (Modifier.isAbstract(hookMethod.getModifiers())) {
            throw new IllegalArgumentException("Cannot hook abstract methods: " + hookMethod.toString());
        }

        boolean newMethod = false;
        CopyOnWriteSortedSet<XC_MethodHook> callbacks;
        synchronized (sHookedMethodCallbacks) {
            callbacks = sHookedMethodCallbacks.get(hookMethod);
            if (callbacks == null) {
                callbacks = new CopyOnWriteSortedSet<>();
                sHookedMethodCallbacks.put(hookMethod, callbacks);
                newMethod = true;
            }
        }
        callbacks.add(callback);

        if (newMethod) {
            XposedHelpers.resolveStaticMethod(hookMethod);
            AdditionalHookInfo additionalInfo = new AdditionalHookInfo(callbacks);
            long slot = WhaleRuntime.hookMethodNative(hookMethod.getDeclaringClass(), hookMethod, additionalInfo);
            if (slot <= 0) {
                throw new IllegalStateException("Failed to hook method: " + hookMethod);
            }
            synchronized (sHookedMethodSlotMap) {
                sHookedMethodSlotMap.put(hookMethod, slot);
            }
        }

        return callback.new Unhook(hookMethod);
    }

    /**
     * Removes the callback for a hooked method/constructor.
     *
     * @param hookMethod The method for which the callback should be removed.
     * @param callback   The reference to the callback as specified in {@link #hookMethod}.
     */
    @SuppressWarnings("all")
    public static void unhookMethod(final Member hookMethod, final XC_MethodHook callback) {
        synchronized (sHookedMethodSlotMap) {
            sHookedMethodSlotMap.remove(hookMethod);
        }
        CopyOnWriteSortedSet<XC_MethodHook> callbacks;
        synchronized (sHookedMethodCallbacks) {
            callbacks = sHookedMethodCallbacks.get(hookMethod);
            if (callbacks == null)
                return;
        }
        callbacks.remove(callback);
    }


    /**
     * Hooks all methods that were declared in the specified class. Inherited
     * methods and constructors are not considered. For constructors, use
     * {@link #hookAllConstructors} instead.
     * <p>
     * AndHook extension function.
     *
     * @param hookClass The class to check for declared methods.
     * @param callback  The callback to be executed when the hooked methods are called.
     * @return A set containing one object for each found method which can be used to unhook it.
     */
    @SuppressWarnings("all")
    public static HashSet<XC_MethodHook.Unhook> hookAllMethods(final Class<?> hookClass,
                                                               final XC_MethodHook callback) {
        final HashSet<XC_MethodHook.Unhook> unhooks = new HashSet<>();
        for (final Member method : hookClass.getDeclaredMethods())
            unhooks.add(hookMethod(method, callback));
        return unhooks;
    }

    /**
     * Hooks all methods with a certain name that were declared in the specified class. Inherited
     * methods and constructors are not considered. For constructors, use
     * {@link #hookAllConstructors} instead.
     *
     * @param hookClass  The class to check for declared methods.
     * @param methodName The name of the method(s) to hook.
     * @param callback   The callback to be executed when the hooked methods are called.
     * @return A set containing one object for each found method which can be used to unhook it.
     */
    @SuppressWarnings("all")
    public static HashSet<XC_MethodHook.Unhook> hookAllMethods(final Class<?> hookClass,
                                                               final String methodName,
                                                               final XC_MethodHook callback) {
        final HashSet<XC_MethodHook.Unhook> unhooks = new HashSet<>();
        for (final Member method : hookClass.getDeclaredMethods())
            if (method.getName().equals(methodName))
                unhooks.add(hookMethod(method, callback));
        return unhooks;
    }

    /**
     * Hook all constructors of the specified class.
     *
     * @param hookClass The class to check for constructors.
     * @param callback  The callback to be executed when the hooked constructors are called.
     * @return A set containing one object for each found constructor which can be used to unhook it.
     */
    @SuppressWarnings("all")
    public static HashSet<XC_MethodHook.Unhook> hookAllConstructors(final Class<?> hookClass,
                                                                    final XC_MethodHook callback) {
        final HashSet<XC_MethodHook.Unhook> unhooks = new HashSet<>();
        for (final Member constructor : hookClass.getDeclaredConstructors())
            unhooks.add(hookMethod(constructor, callback));
        return unhooks;
    }

    /**
     * This method is called as a replacement for hooked methods.
     */
    public static Object handleHookedMethod(Member method, long slot, Object additionalInfoObj,
                                            Object thisObject, Object[] args) throws Throwable {
        AdditionalHookInfo additionalInfo = (AdditionalHookInfo) additionalInfoObj;

        if (disableHooks) {
            try {
                return invokeOriginalMethod(slot, thisObject, args);
            } catch (InvocationTargetException e) {
                throw e.getCause();
            }
        }

        Object[] callbacksSnapshot = additionalInfo.callbacks.getSnapshot();
        final int callbacksLength = callbacksSnapshot.length;
        if (callbacksLength == 0) {
            try {
                return invokeOriginalMethod(slot, thisObject, args);
            } catch (InvocationTargetException e) {
                throw e.getCause();
            }
        }

        MethodHookParam param = new MethodHookParam();
        param.method = method;
        param.thisObject = thisObject;
        param.args = args;

        // call "before method" callbacks
        int beforeIdx = 0;
        do {
            try {
                ((XC_MethodHook) callbacksSnapshot[beforeIdx]).beforeHookedMethod(param);
            } catch (Throwable t) {
                XposedBridge.log(t);

                // reset result (ignoring what the unexpectedly exiting callback did)
                param.setResult(null);
                param.returnEarly = false;
                continue;
            }

            if (param.returnEarly) {
                // skip remaining "before" callbacks and corresponding "after" callbacks
                beforeIdx++;
                break;
            }
        } while (++beforeIdx < callbacksLength);

        // call original method if not requested otherwise
        if (!param.returnEarly) {
            try {
                param.setResult(invokeOriginalMethod(slot,
                        param.thisObject, param.args));
            } catch (InvocationTargetException e) {
                param.setThrowable(e.getCause());
            }
        }

        // call "after method" callbacks
        int afterIdx = beforeIdx - 1;
        do {
            Object lastResult = param.getResult();
            Throwable lastThrowable = param.getThrowable();

            try {
                ((XC_MethodHook) callbacksSnapshot[afterIdx]).afterHookedMethod(param);
            } catch (Throwable t) {
                XposedBridge.log(t);

                // reset to last result (ignoring what the unexpectedly exiting callback did)
                if (lastThrowable == null)
                    param.setResult(lastResult);
                else
                    param.setThrowable(lastThrowable);
            }
        } while (--afterIdx >= 0);

        // return
        if (param.hasThrowable())
            throw param.getThrowable();
        else
            return param.getResult();
    }


    /**
     * Basically the same as {@link Method#invoke}, but calls the original method
     * as it was before the interception by Xposed. Also, access permissions are not checked.
     * <p>
     * <p class="caution">There are very few cases where this method is needed. A common mistake is
     * to replace a method and then invoke the original one based on dynamic conditions. This
     * creates overhead and skips further hooks by other modules. Instead, just hook (don't replace)
     * the method and call {@code param.setResult(null)} in {@link XC_MethodHook#beforeHookedMethod}
     * if the original method should be skipped.
     */
    @SuppressWarnings("unused")
    public static Object invokeOriginalMethod(final Member method, final Object thisObject,
                                              final Object[] args)
            throws NullPointerException, IllegalAccessException, IllegalArgumentException, InvocationTargetException {
        long slot = sHookedMethodSlotMap.get(method);
        return WhaleRuntime.invokeOriginalMethodNative(slot, thisObject, args);
    }

    public static Object invokeOriginalMethod(final long slot, final Object thisObject,
                                              final Object[] args)
            throws NullPointerException, IllegalAccessException, IllegalArgumentException, InvocationTargetException {
        return WhaleRuntime.invokeOriginalMethodNative(slot, thisObject, args);
    }

    public static final class CopyOnWriteSortedSet<E> {
        private transient volatile Object[] elements = EMPTY_ARRAY;

        @SuppressWarnings("UnusedReturnValue")
        public synchronized boolean add(final E e) {
            final int index = indexOf(e);
            if (index >= 0)
                return false;

            final Object[] newElements = new Object[elements.length + 1];
            System.arraycopy(elements, 0, newElements, 0, elements.length);
            newElements[elements.length] = e;
            Arrays.sort(newElements);
            elements = newElements;
            return true;
        }

        @SuppressWarnings("all")
        public synchronized boolean remove(final E e) {
            final int index = indexOf(e);
            if (index == -1)
                return false;

            final Object[] newElements = new Object[elements.length - 1];
            System.arraycopy(elements, 0, newElements, 0, index);
            System.arraycopy(elements, index + 1, newElements, index, elements.length - index - 1);
            elements = newElements;
            return true;
        }

        private int indexOf(final Object o) {
            for (int i = 0; i < elements.length; ++i) {
                if (o.equals(elements[i]))
                    return i;
            }
            return -1;
        }

        public Object[] getSnapshot() {
            return elements;
        }
    }

    private static final class AdditionalHookInfo {
        final CopyOnWriteSortedSet<XC_MethodHook> callbacks;

        public AdditionalHookInfo(CopyOnWriteSortedSet<XC_MethodHook> callbacks) {
            this.callbacks = callbacks;
        }
    }
}