package com.carto.utils;

/**
 * Annotation to notify that class name and its members should be kept intact and not obfuscated.
 * It is needed as reflection is used when accessing SDK Java classes from native SDK code.
 * If you are using ProGuard, then add the following rules to your 'proguard-rules.pro' file:
 * -keep class com.carto.utils.DontObfuscate
 * -keep enum com.carto.**
 * -keep @com.carto.utils.DontObfuscate class ** { *; }
 */
@java.lang.annotation.Retention(java.lang.annotation.RetentionPolicy.CLASS)
public @interface DontObfuscate {
}
