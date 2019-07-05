package com.lody.whale.enity;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface HookModule {
    String name();
    String targetPkgName() default "all";//"all" means it will be loaded into every app.
}
