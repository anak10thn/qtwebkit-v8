# -------------------------------------------------------------------
# Project file for WebKitTestRunner binary (WTR)
#
# See 'Tools/qmake/README' for an overview of the build system
# -------------------------------------------------------------------

TEMPLATE = app
TARGET = WebKitTestRunner

HEADERS += \
    EventSenderProxy.h \
    PlatformWebView.h \
    StringFunctions.h \
    TestController.h \
    TestInvocation.h

SOURCES += \
    qt/main.cpp \
    qt/EventSenderProxyQt.cpp \
    qt/PlatformWebViewQt.cpp \
    qt/TestControllerQt.cpp \
    qt/TestInvocationQt.cpp \
    TestController.cpp \
    TestInvocation.cpp

DESTDIR = $${ROOT_BUILD_DIR}/bin

QT = core gui widgets network declarative testlib quick quick-private webkit

load(features)

WEBKIT += wtf webkit2
v8: WEBKIT += v8jscapishim
else: WEBKIT += javascriptcore

DEFINES += USE_SYSTEM_MALLOC=1

PREFIX_HEADER = WebKitTestRunnerPrefix.h
*-g++*:QMAKE_CXXFLAGS += "-include $$PREFIX_HEADER"
*-clang*:QMAKE_CXXFLAGS += "-include $$PREFIX_HEADER"

RESOURCES = qt/WebKitTestRunner.qrc
