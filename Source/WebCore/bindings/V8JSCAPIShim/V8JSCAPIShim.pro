TEMPLATE = lib
TARGET = v8jscapishim
CONFIG += staticlib

include(V8JSCAPIShim.pri)

VPATH += $$PWD/JavaScriptCore

WEBKIT += wtf
QT = core v8-private

SOURCES += \
    JavaScriptCore/ClassWrapper.cpp \
    JavaScriptCore/JSBase.cpp \
    JavaScriptCore/JSContextRef.cpp \
    JavaScriptCore/JSObjectRef.cpp \
    JavaScriptCore/JSStringRef.cpp \
    JavaScriptCore/JSValueRef.cpp

HEADERS += \
    JavaScriptCore/APICast.h \
    JavaScriptCore/ClassWrapper.h \
    JavaScriptCore/JavaScript.h \
    JavaScriptCore/JSBase.h \
    JavaScriptCore/JSContextRef.h \
    JavaScriptCore/JSObjectRef.h \
    JavaScriptCore/JSStringRef.h \
    JavaScriptCore/JSValueRef.h \
    JavaScriptCore/OpaqueJSString.h \
    JavaScriptCore/V8JSCAPIShimMacros.h
