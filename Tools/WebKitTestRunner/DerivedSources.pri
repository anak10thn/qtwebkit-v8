# -------------------------------------------------------------------
# Derived sources for WebKitTestRunner
#
# See 'Tools/qmake/README' for an overview of the build system
# -------------------------------------------------------------------

TEMPLATE = derived

# Make sure forwarded headers needed by this project are present
fwheader_generator.commands = perl $${ROOT_WEBKIT_DIR}/Source/WebKit2/Scripts/generate-forwarding-headers.pl $${ROOT_WEBKIT_DIR}/Tools/WebKitTestRunner $${ROOT_BUILD_DIR}/Source/include qt
fwheader_generator.depends  = $${ROOT_WEBKIT_DIR}/Source/WebKit2/Scripts/generate-forwarding-headers.pl
v8:fwheader_generator.commands += v8
else: fwheader_generator.commands += js JavaScriptCore/API
GENERATORS += fwheader_generator
