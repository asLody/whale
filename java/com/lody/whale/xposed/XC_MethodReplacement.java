package com.lody.whale.xposed;

import com.lody.whale.xposed.callbacks.XCallback;

/**
 * A special case of {@link XC_MethodHook} which completely replaces the original method.
 */
@SuppressWarnings({"unused", "WeakerAccess"})
public abstract class XC_MethodReplacement extends XC_MethodHook {
    /**
     * Creates a new callback with default priority.
     */
    public XC_MethodReplacement() {
        super();
    }

    /**
     * Creates a new callback with a specific priority.
     *
     * @param priority See {@link XCallback#priority}.
     */
    public XC_MethodReplacement(final int priority) {
        super(priority);
    }

    @Override
    protected final void beforeHookedMethod(final MethodHookParam param) throws Throwable {
        try {
            param.setResult(replaceHookedMethod(param));
        } catch (final Throwable t) {
            param.setThrowable(t);
        }
    }

    /**
     * @hide
     */
    @Override
    @SuppressWarnings("all")
    protected final void afterHookedMethod(final MethodHookParam param) throws Throwable {
    }

    /**
     * Shortcut for replacing a method completely. Whatever is returned/thrown here is taken
     * instead of the result of the original method (which will not be called).
     * <p>
     * <p>Note that implementations shouldn't call {@code super(param)}, it's not necessary.
     *
     * @param param Information about the method call.
     * @throws Throwable Anything that is thrown by the callback will be passed on to the original caller.
     */
    @SuppressWarnings("all")
    protected abstract Object replaceHookedMethod(final MethodHookParam param) throws Throwable;

    /**
     * Predefined callback that skips the method without replacements.
     */
    public static final XC_MethodReplacement DO_NOTHING = new XC_MethodReplacement(PRIORITY_HIGHEST * 2) {
        @Override
        protected Object replaceHookedMethod(final MethodHookParam param) throws Throwable {
            return null;
        }
    };

    /**
     * Creates a callback which always returns a specific value.
     *
     * @param result The value that should be returned to callers of the hooked method.
     */
    public static XC_MethodReplacement returnConstant(final Object result) {
        return returnConstant(XCallback.PRIORITY_DEFAULT, result);
    }

    /**
     * Like {@link #returnConstant(Object)}, but allows to specify a priority for the callback.
     *
     * @param priority See {@link XCallback#priority}.
     * @param result   The value that should be returned to callers of the hooked method.
     */
    public static XC_MethodReplacement returnConstant(final int priority, final Object result) {
        return new XC_MethodReplacement(priority) {
            @Override
            protected Object replaceHookedMethod(final MethodHookParam param) throws Throwable {
                return result;
            }
        };
    }

}