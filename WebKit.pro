# -------------------------------------------------------------------
# Root project file, used to load WebKit in Qt Creator and for
# building QtWebKit.
#
# See 'Tools/qmake/README' for an overview of the build system
# -------------------------------------------------------------------

TEMPLATE = subdirs
CONFIG += ordered

load(features)

QMAKEPATH = $$(QMAKEPATH)
isEmpty(QMAKEPATH)|!exists($${QMAKEPATH}/mkspecs) {
    error("The environment variable QMAKEPATH needs to point to $WEBKITSRC/Tools/qmake")
    # Otherwise we won't pick up the feature prf files needed for the build
}

WTF.file = Source/WTF/WTF.pro
WTF.makefile = Makefile.WTF
SUBDIRS += WTF

!v8 {
    JavaScriptCore.file = Source/JavaScriptCore/JavaScriptCore.pro
    JavaScriptCore.makefile = Makefile.JavaScriptCore
    SUBDIRS += JavaScriptCore
}

v8 {
    v8jscapishim.file = Source/WebCore/bindings/V8JSCAPIShim/V8JSCAPIShim.pro
    v8jscapishim.makefile = Makefile.v8jscapishim
    SUBDIRS += v8jscapishim
}

WebCore.file = Source/WebCore/WebCore.pro
WebCore.makefile = Makefile.WebCore
SUBDIRS += WebCore

!CONFIG(no_webkit2) {
    webkit2.file = Source/WebKit2/WebKit2.pro
    webkit2.makefile = Makefile.WebKit2
    SUBDIRS += webkit2
}

QtWebKit.file = Source/QtWebKit.pro
QtWebKit.makefile = Makefile.QtWebKit
SUBDIRS += QtWebKit

SUBDIRS += Tools
