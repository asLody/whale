package com.lody.whale.xposed;

import java.lang.reflect.Member;

import com.lody.whale.xposed.callbacks.XCallback;

/**
 * Callback class for method hooks.
 * <p>
 * <p>Usually, anonymous subclasses of this class are created which override
 * {@link #beforeHookedMethod} and/or {@link #afterHookedMethod}.
 */
public class XC_MethodHook extends XCallback {
    /**
     * Creates a new callback with default priority.
     */
    protected XC_MethodHook() {
        super();
    }

    /**
     * Creates a new callback with a specific priority.
     * <p>
     * <p class="note">Note that {@link #afterHookedMethod} will be called in reversed order, i.e.
     * the callback with the highest priority will be called last. This way, the callback has the
     * final control over the return value. {@link #beforeHookedMethod} is called as usual, i.e.
     * highest priority first.
     *
     * @param priority See {@link XCallback#priority}.
     */
    XC_MethodHook(final int priority) {
        super(priority);
    }

    /**
     * Called before the invocation of the method.
     * <p>
     * <p>
     * You can use {@link MethodHookParam#setResult} and
     * {@link MethodHookParam#setThrowable} to prevent the original method from
     * being called.
     * <p>
     * <p>
     * Note that implementations shouldn't call {@code super(param)}, it's not
     * necessary.
     *
     * @param param Information about the method call.
     * @throws Throwable Everything the callback throws is caught and logged.
     */
    protected void beforeHookedMethod(final MethodHookParam param) throws Throwable {
    }

    /**
     * Called after the invocation of the method.
     * <p>
     * <p>
     * You can use {@link MethodHookParam#setResult} and
     * {@link MethodHookParam#setThrowable} to modify the return value of the
     * original method.
     * <p>
     * <p>
     * Note that implementations shouldn't call {@code super(param)}, it's not
     * necessary.
     *
     * @param param Information about the method call.
     * @throws Throwable Everything the callback throws is caught and logged.
     */
    protected void afterHookedMethod(final MethodHookParam param) throws Throwable {
    }

    /**
     * Wraps information about the method call and allows to influence it.
     */
    @SuppressWarnings({"unused", "WeakerAccess"})
    public static final class MethodHookParam extends Param {
        /**
         * Backup method slot.
         */
        public int slot;

        /**
         * The hooked method/constructor.
         */
        public Member method;

        /**
         * The {@code this} reference for an instance method, or {@code null} for static methods.
         */
        public Object thisObject;

        /**
         * Arguments to the method call.
         */
        public Object[] args;

        private Object result = null;
        private Throwable throwable = null;
        /* package */ boolean returnEarly = false;

        /**
         * Returns the result of the method call.
         */
        public Object getResult() {
            return result;
        }

        /**
         * Modify the result of the method call.
         * <p>
         * <p>If called from {@link #beforeHookedMethod}, it prevents the call to the original method.
         */
        public void setResult(final Object result) {
            this.result = result;
            this.throwable = null;
            this.returnEarly = true;
        }

        /**
         * Returns the {@link Throwable} thrown by the method, or {@code null}.
         */
        public Throwable getThrowable() {
            return throwable;
        }

        /**
         * Returns true if an exception was thrown by the method.
         */
        public boolean hasThrowable() {
            return throwable != null;
        }

        /**
         * Modify the exception thrown of the method call.
         * <p>
         * <p>If called from {@link #beforeHookedMethod}, it prevents the call to the original method.
         */
        public void setThrowable(final Throwable throwable) {
            this.throwable = throwable;
            this.result = null;
            this.returnEarly = true;
        }

        /**
         * Returns the result of the method call, or throws the Throwable caused by it.
         */
        public Object getResultOrThrowable() throws Throwable {
            if (throwable != null)
                throw throwable;
            return result;
        }
    }

    /**
     * An object with which the method/constructor can be unhooked.
     */
    @SuppressWarnings("all")
    public final class Unhook {
        private final Member hookMethod;

        Unhook(final Member hookMethod) {
            this.hookMethod = hookMethod;
        }

        /**
         * Returns the method/constructor that has been hooked.
         */
        public Member getHookedMethod() {
            return this.hookMethod;
        }

        public XC_MethodHook getCallback() {
            return XC_MethodHook.this;
        }

        public void unhook() {
            XposedBridge.unhookMethod(this.hookMethod, XC_MethodHook.this);
        }
    }
}
