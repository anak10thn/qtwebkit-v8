/*
 * Copyright (C) 2011 ProFUSION Embedded Systems
 * Copyright (C) 2011 Samsung Electronics
 * Copyright (C) 2012 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND ITS CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "DumpRenderTreeChrome.h"

#include "DumpRenderTree.h"
#include "DumpRenderTreeView.h"
#include "EditingCallbacks.h"
#include "EventSender.h"
#include "GCController.h"
#include "KURL.h"
#include "LayoutTestController.h"
#include "NotImplemented.h"
#include "TextInputController.h"
#include "WebCoreSupport/DumpRenderTreeSupportEfl.h"
#include "WebCoreTestSupport.h"
#include "WorkQueue.h"
#include "ewk_private.h" // FIXME: create some WebCoreSupport/DumpRenderTree.cpp instead

#include <EWebKit.h>
#include <Ecore.h>
#include <Eina.h>
#include <Evas.h>
#include <cstdio>
#include <wtf/NotFound.h>

using namespace WebCore;

HashMap<unsigned long, CString> DumpRenderTreeChrome::m_dumpAssignedUrls;
Evas_Object* DumpRenderTreeChrome::m_provisionalLoadFailedFrame = 0;
Ewk_Intent_Request* DumpRenderTreeChrome::m_currentIntentRequest = 0;

PassOwnPtr<DumpRenderTreeChrome> DumpRenderTreeChrome::create(Evas* evas)
{
    OwnPtr<DumpRenderTreeChrome> chrome = adoptPtr(new DumpRenderTreeChrome(evas));

    if (!chrome->initialize())
        return nullptr;

    return chrome.release();
}

DumpRenderTreeChrome::DumpRenderTreeChrome(Evas* evas)
    : m_mainView(0)
    , m_mainFrame(0)
    , m_evas(evas)
    , m_gcController(adoptPtr(new GCController))
{
}

DumpRenderTreeChrome::~DumpRenderTreeChrome()
{
}

Evas_Object* DumpRenderTreeChrome::createNewWindow()
{
    Evas_Object* newView = createView();

    ewk_view_setting_scripts_can_open_windows_set(newView, EINA_TRUE);
    ewk_view_setting_scripts_can_close_windows_set(newView, EINA_TRUE);

    m_extraViews.append(newView);

    return newView;
}

Evas_Object* DumpRenderTreeChrome::createView() const
{
    Evas_Object* view = drtViewAdd(m_evas);
    if (!view)
        return 0;

    ewk_view_theme_set(view, DATA_DIR"/default.edj");

    evas_object_smart_callback_add(view, "load,resource,failed", onResourceLoadFailed, 0);
    evas_object_smart_callback_add(view, "load,resource,finished", onResourceLoadFinished, 0);
    evas_object_smart_callback_add(view, "load,started", onLoadStarted, 0);
    evas_object_smart_callback_add(view, "window,object,cleared", onWindowObjectCleared, m_gcController.get());
    evas_object_smart_callback_add(view, "statusbar,text,set", onStatusbarTextSet, 0);
    evas_object_smart_callback_add(view, "load,document,finished", onDocumentLoadFinished, 0);
    evas_object_smart_callback_add(view, "resource,request,new", onNewResourceRequest, 0);
    evas_object_smart_callback_add(view, "resource,request,willsend", onWillSendRequest, 0);
    evas_object_smart_callback_add(view, "resource,response,received", onResponseReceived, 0);
    evas_object_smart_callback_add(view, "onload,event", onWebViewOnloadEvent, 0);
    evas_object_smart_callback_add(view, "mixedcontent,run", onInsecureContentRun, 0);
    evas_object_smart_callback_add(view, "mixedcontent,displayed", onInsecureContentDisplayed, 0);
    evas_object_smart_callback_add(view, "frame,created", onFrameCreated, 0);

    connectEditingCallbacks(view);

    Evas_Object* mainFrame = ewk_view_frame_main_get(view);
    evas_object_smart_callback_add(mainFrame, "icon,changed", onFrameIconChanged, 0);
    evas_object_smart_callback_add(mainFrame, "intent,new", onFrameIntentNew, 0);
    evas_object_smart_callback_add(mainFrame, "load,provisional", onFrameProvisionalLoad, 0);
    evas_object_smart_callback_add(mainFrame, "load,provisional,failed", onFrameProvisionalLoadFailed, 0);
    evas_object_smart_callback_add(mainFrame, "load,committed", onFrameLoadCommitted, 0);
    evas_object_smart_callback_add(mainFrame, "load,finished", onFrameLoadFinished, 0);
    evas_object_smart_callback_add(mainFrame, "load,error", onFrameLoadError, 0);
    evas_object_smart_callback_add(mainFrame, "redirect,cancelled", onFrameRedirectCancelled, 0);
    evas_object_smart_callback_add(mainFrame, "redirect,load,provisional", onFrameRedirectForProvisionalLoad, 0);
    evas_object_smart_callback_add(mainFrame, "redirect,requested", onFrameRedirectRequested, 0);
    evas_object_smart_callback_add(mainFrame, "title,changed", onFrameTitleChanged, 0);
    evas_object_smart_callback_add(mainFrame, "xss,detected", onDidDetectXSS, 0);

    return view;
}

void DumpRenderTreeChrome::removeWindow(Evas_Object* view)
{
    const size_t pos = m_extraViews.find(view);

    if (pos == notFound)
        return;

    m_extraViews.remove(pos);
    evas_object_del(view);
}

bool DumpRenderTreeChrome::initialize()
{
    DumpRenderTreeSupportEfl::setMockScrollbarsEnabled(true);

    m_mainView = createView();
    if (!m_mainView)
        return false;

    ewk_view_theme_set(m_mainView, DATA_DIR"/default.edj");

    evas_object_name_set(m_mainView, "m_mainView");
    evas_object_move(m_mainView, 0, 0);
    evas_object_resize(m_mainView, 800, 600);
    evas_object_layer_set(m_mainView, EVAS_LAYER_MAX);
    evas_object_show(m_mainView);
    evas_object_focus_set(m_mainView, EINA_TRUE);

    m_mainFrame = ewk_view_frame_main_get(m_mainView);

    char* drtTemp = getenv("DUMPRENDERTREE_TEMP");
    if (drtTemp) {
        char path[2048];
        eina_str_join(path, sizeof(path), '/', drtTemp, "LocalStorage");
        ewk_settings_local_storage_path_set(path);
        eina_str_join(path, sizeof(path), '/', drtTemp, "Databases");
        ewk_settings_web_database_path_set(path);
        eina_str_join(path, sizeof(path), '/', drtTemp, "Applications");
        ewk_settings_application_cache_path_set(path);
    }

    return true;
}

const Vector<Evas_Object*>& DumpRenderTreeChrome::extraViews() const
{
    return m_extraViews;
}

void DumpRenderTreeChrome::clearExtraViews()
{
    Vector<Evas_Object*>::iterator it = m_extraViews.begin();
    for (; it != m_extraViews.end(); ++it)
        evas_object_del(*it);
    m_extraViews.clear();
}

Evas_Object* DumpRenderTreeChrome::mainFrame() const
{
    return m_mainFrame;
}

Evas_Object* DumpRenderTreeChrome::mainView() const
{
    return m_mainView;
}

Ewk_Intent_Request* DumpRenderTreeChrome::currentIntentRequest() const
{
    return m_currentIntentRequest;
}

void DumpRenderTreeChrome::resetDefaultsToConsistentValues()
{
    ewk_settings_icon_database_clear();
    ewk_settings_icon_database_path_set(0);

    ewk_web_database_remove_all();
    ewk_settings_web_database_default_quota_set(5 * 1024 * 1024);

    ewk_settings_memory_cache_clear();
    ewk_settings_application_cache_clear();

    ewk_view_setting_private_browsing_set(mainView(), EINA_FALSE);
    ewk_view_setting_spatial_navigation_set(mainView(), EINA_FALSE);
    ewk_view_setting_enable_frame_flattening_set(mainView(), EINA_FALSE);
    ewk_view_setting_application_cache_set(mainView(), EINA_TRUE);
    ewk_view_setting_enable_scripts_set(mainView(), EINA_TRUE);
    ewk_view_font_family_name_set(mainView(), EWK_FONT_FAMILY_STANDARD, "Times");
    ewk_view_font_family_name_set(mainView(), EWK_FONT_FAMILY_MONOSPACE, "Courier");
    ewk_view_font_family_name_set(mainView(), EWK_FONT_FAMILY_SERIF, "Times");
    ewk_view_font_family_name_set(mainView(), EWK_FONT_FAMILY_SANS_SERIF, "Helvetica");
    ewk_view_font_family_name_set(mainView(), EWK_FONT_FAMILY_CURSIVE, "cursive");
    ewk_view_font_family_name_set(mainView(), EWK_FONT_FAMILY_FANTASY, "fantasy");
    ewk_view_setting_font_default_size_set(mainView(), 16);
    ewk_view_setting_font_monospace_size_set(mainView(), 13);
    ewk_view_setting_font_minimum_size_set(mainView(), 0);
    ewk_view_setting_caret_browsing_set(mainView(), EINA_FALSE);
    ewk_view_setting_page_cache_set(mainView(), EINA_FALSE);
    ewk_view_setting_enable_auto_resize_window_set(mainView(), EINA_TRUE);
    ewk_view_setting_enable_plugins_set(mainView(), EINA_TRUE);
    ewk_view_setting_scripts_can_open_windows_set(mainView(), EINA_TRUE);
    ewk_view_setting_scripts_can_close_windows_set(mainView(), EINA_TRUE);
    ewk_view_setting_auto_load_images_set(mainView(), EINA_TRUE);
    ewk_view_setting_user_stylesheet_set(mainView(), 0);
    ewk_view_setting_enable_xss_auditor_set(browser->mainView(), EINA_TRUE);
    ewk_view_setting_minimum_timer_interval_set(browser->mainView(), 0.010); // 10 milliseconds (DOMTimer::s_minDefaultTimerInterval)
    ewk_view_setting_enable_webgl_set(mainView(), EINA_TRUE);
    ewk_view_setting_enable_hyperlink_auditing_set(mainView(), EINA_FALSE);
    ewk_view_setting_include_links_in_focus_chain_set(mainView(), EINA_TRUE);
    ewk_view_setting_scripts_can_access_clipboard_set(mainView(), EINA_TRUE);

    ewk_view_zoom_set(mainView(), 1.0, 0, 0);
    ewk_view_scale_set(mainView(), 1.0, 0, 0);
    ewk_view_text_zoom_set(mainView(), 1.0);
    ewk_view_visibility_state_set(mainView(), EWK_PAGE_VISIBILITY_STATE_VISIBLE, true);

    ewk_history_clear(ewk_view_history_get(mainView()));

    ewk_cookies_clear();
    ewk_cookies_policy_set(EWK_COOKIE_JAR_ACCEPT_NO_THIRD_PARTY);

    ewk_security_policy_whitelist_origin_reset();

    DumpRenderTreeSupportEfl::clearFrameName(mainFrame());
    DumpRenderTreeSupportEfl::clearOpener(mainFrame());
    DumpRenderTreeSupportEfl::clearUserScripts(mainView());
    DumpRenderTreeSupportEfl::clearUserStyleSheets(mainView());
    DumpRenderTreeSupportEfl::setInteractiveFormValidationEnabled(mainView(), true);
    DumpRenderTreeSupportEfl::setValidationMessageTimerMagnification(mainView(), -1);
    DumpRenderTreeSupportEfl::setAuthorAndUserStylesEnabled(mainView(), true);
    DumpRenderTreeSupportEfl::setCSSGridLayoutEnabled(mainView(), false);
    DumpRenderTreeSupportEfl::setSmartInsertDeleteEnabled(mainView(), false);
    DumpRenderTreeSupportEfl::setSelectTrailingWhitespaceEnabled(mainView(), false);
    DumpRenderTreeSupportEfl::setDefersLoading(mainView(), false);
    DumpRenderTreeSupportEfl::setLoadsSiteIconsIgnoringImageLoadingSetting(mainView(), false);
    DumpRenderTreeSupportEfl::setSerializeHTTPLoads(false);
    DumpRenderTreeSupportEfl::setDeadDecodedDataDeletionInterval(0);

    if (m_currentIntentRequest) {
        ewk_intent_request_unref(m_currentIntentRequest);
        m_currentIntentRequest = 0;
    }
}

static CString pathSuitableForTestResult(const char* uriString)
{
    if (!uriString)
        return CString();

    KURL uri = KURL(ParsedURLString, uriString);

    if (!uri.isLocalFile())
        return uri.string().utf8();

    String pathString = uri.path();
    size_t indexBaseName = pathString.reverseFind('/');
    String baseName;
    if (indexBaseName == notFound)
        baseName = pathString;
    else
        baseName = pathString.substring(indexBaseName + 1);

    String dirName;
    if (indexBaseName != notFound) {
        size_t indexDirName = pathString.reverseFind('/', indexBaseName - 1);
        if (indexDirName != notFound)
            dirName = pathString.substring(indexDirName + 1, indexBaseName - indexDirName - 1);
    }

    String ret = dirName + "/" + baseName;
    return ret.utf8();
}

static CString urlSuitableForTestResult(const char* uriString)
{
    KURL uri = KURL(ParsedURLString, uriString);
    if (!uri.isLocalFile())
        return CString(uriString);

    unsigned startIndex = uri.pathAfterLastSlash();
    return uri.string().substring(startIndex).utf8();
}

static CString descriptionSuitableForTestResult(Ewk_Frame_Resource_Request* request)
{
    String ret = "<NSURLRequest URL ";
    ret += pathSuitableForTestResult(request->url).data();
    ret += ", main document URL ";
    ret += urlSuitableForTestResult(request->first_party).data();
    ret += ", http method ";
    ret += request->http_method ? String(request->http_method) : "(none)";
    ret += ">";

    return ret.utf8();
}

static CString descriptionSuitableForTestResult(const Ewk_Frame_Resource_Response* response)
{
    if (!response)
        return CString("(null)");

    String ret = "<NSURLResponse ";
    ret += pathSuitableForTestResult(response->url).data();
    ret += ", http status code ";
    ret += String::number(response->status_code);
    ret += ">";

    return ret.utf8();
}

static CString descriptionSuitableForTestResult(Ewk_Frame_Load_Error* error)
{
    String ret = "<NSError domain ";
    ret += error->domain;
    ret += ", code ";
    ret += String::number(error->code);
    if (error->failing_url && *error->failing_url != '\0') {
        ret += ", failing URL \"";
        ret += error->failing_url;
        ret += "\"";
    }
    ret += ">";

    return ret.utf8();
}

// Smart Callbacks
// ---------------

void DumpRenderTreeChrome::onWindowObjectCleared(void* userData, Evas_Object*, void* eventInfo)
{
    Ewk_Window_Object_Cleared_Event* objectClearedInfo = static_cast<Ewk_Window_Object_Cleared_Event*>(eventInfo);
    JSValueRef exception = 0;
    ASSERT(gLayoutTestController);

    GCController* gcController = static_cast<GCController*>(userData);
    ASSERT(gcController);

    gLayoutTestController->makeWindowObject(objectClearedInfo->context, objectClearedInfo->windowObject, &exception);
    ASSERT(!exception);

    gcController->makeWindowObject(objectClearedInfo->context, objectClearedInfo->windowObject, &exception);
    ASSERT(!exception);

    JSRetainPtr<JSStringRef> controllerName(Adopt, JSStringCreateWithUTF8CString("eventSender"));
    JSObjectSetProperty(objectClearedInfo->context, objectClearedInfo->windowObject,
                        controllerName.get(),
                        makeEventSender(objectClearedInfo->context, !DumpRenderTreeSupportEfl::frameParent(objectClearedInfo->frame)),
                        kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, 0);

    JSRetainPtr<JSStringRef> textInputControllerName(JSStringCreateWithUTF8CString("textInputController"));
    JSObjectSetProperty(objectClearedInfo->context, objectClearedInfo->windowObject,
                        textInputControllerName.get(),
                        makeTextInputController(objectClearedInfo->context),
                        kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, 0);

    WebCoreTestSupport::injectInternalsObject(objectClearedInfo->context);
}

void DumpRenderTreeChrome::onLoadStarted(void*, Evas_Object* view, void* eventInfo)
{
    Evas_Object* frame = static_cast<Evas_Object*>(eventInfo);

    // Make sure we only set this once per test. If it gets cleared, and then set again, we might
    // end up doing two dumps for one test.
    if (!topLoadingFrame && !done)
        topLoadingFrame = frame;
}

Eina_Bool DumpRenderTreeChrome::processWork(void*)
{
    if (WorkQueue::shared()->processWork() && !gLayoutTestController->waitToDump())
        dump();

    return ECORE_CALLBACK_CANCEL;
}

void DumpRenderTreeChrome::topLoadingFrameLoadFinished()
{
    topLoadingFrame = 0;

    WorkQueue::shared()->setFrozen(true);
    if (gLayoutTestController->waitToDump())
        return;

    if (WorkQueue::shared()->count())
        ecore_idler_add(processWork, 0 /*frame*/);
    else
        dump();
}

void DumpRenderTreeChrome::onStatusbarTextSet(void*, Evas_Object*, void* eventInfo)
{
    if (!gLayoutTestController->dumpStatusCallbacks())
        return;

    const char* statusbarText = static_cast<const char*>(eventInfo);
    printf("UI DELEGATE STATUS CALLBACK: setStatusText:%s\n", statusbarText);
}

void DumpRenderTreeChrome::onFrameIconChanged(void*, Evas_Object* frame, void*)
{
    if (!done && gLayoutTestController->dumpIconChanges()) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - didChangeIcons\n", frameName.utf8().data());
    }
}

void DumpRenderTreeChrome::onFrameTitleChanged(void*, Evas_Object* frame, void* eventInfo)
{
    const Ewk_Text_With_Direction* titleText = static_cast<const Ewk_Text_With_Direction*>(eventInfo);

    if (!done && gLayoutTestController->dumpFrameLoadCallbacks()) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - didReceiveTitle: %s\n", frameName.utf8().data(), (titleText && titleText->string) ? titleText->string : "");
    }

    if (!done && gLayoutTestController->dumpTitleChanges())
        printf("TITLE CHANGED: %s\n", (titleText && titleText->string) ? titleText->string : "");
}

void DumpRenderTreeChrome::onDocumentLoadFinished(void*, Evas_Object*, void* eventInfo)
{
    const Evas_Object* frame = static_cast<Evas_Object*>(eventInfo);
    const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));

    if (!done && gLayoutTestController->dumpFrameLoadCallbacks())
        printf("%s - didFinishDocumentLoadForFrame\n", frameName.utf8().data());
    else if (!done) {
        const unsigned pendingFrameUnloadEvents = DumpRenderTreeSupportEfl::pendingUnloadEventCount(frame);
        if (pendingFrameUnloadEvents)
            printf("%s - has %u onunload handler(s)\n", frameName.utf8().data(), pendingFrameUnloadEvents);
    }
}

void DumpRenderTreeChrome::onWillSendRequest(void*, Evas_Object*, void* eventInfo)
{
    Ewk_Frame_Resource_Messages* messages = static_cast<Ewk_Frame_Resource_Messages*>(eventInfo);

    if (!done && gLayoutTestController->dumpResourceLoadCallbacks())
        printf("%s - willSendRequest %s redirectResponse %s\n",
               m_dumpAssignedUrls.contains(messages->request->identifier) ? m_dumpAssignedUrls.get(messages->request->identifier).data() : "<unknown>",
               descriptionSuitableForTestResult(messages->request).data(),
               descriptionSuitableForTestResult(messages->redirect_response).data());

    if (!done && gLayoutTestController->willSendRequestReturnsNull()) {
        // As requested by the LayoutTestController, don't perform the request.
        messages->request->url = 0;
        return;
    }

    if (!done && gLayoutTestController->willSendRequestReturnsNullOnRedirect() && messages->redirect_response) {
        printf("Returning null for this redirect\n");
        messages->request->url = 0;
        return;
    }

    KURL url = KURL(ParsedURLString, messages->request->url);

    if (url.isValid()
        && url.protocolIsInHTTPFamily()
        && url.host() != "127.0.0.1"
        && url.host() != "255.255.255.255"
        && url.host().lower() != "localhost") {
        printf("Blocked access to external URL %s\n", messages->request->url);
        messages->request->url = 0;
        return;
    }

    const std::string& destination = gLayoutTestController->redirectionDestinationForURL(url.string().utf8().data());
    if (destination.length())
        messages->request->url = strdup(destination.c_str());
}

void DumpRenderTreeChrome::onWebViewOnloadEvent(void*, Evas_Object*, void* eventInfo)
{
    const Evas_Object* frame = static_cast<Evas_Object*>(eventInfo);

    if (!done && gLayoutTestController->dumpFrameLoadCallbacks()) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - didHandleOnloadEventsForFrame\n", frameName.utf8().data());
    }
}

void DumpRenderTreeChrome::onInsecureContentRun(void*, Evas_Object*, void*)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks())
        printf("didRunInsecureContent\n");
}

void DumpRenderTreeChrome::onInsecureContentDisplayed(void*, Evas_Object*, void*)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks())
        printf("didDisplayInsecureContent\n");
}

void DumpRenderTreeChrome::onFrameCreated(void*, Evas_Object*, void* eventInfo)
{
    Evas_Object* frame = static_cast<Evas_Object*>(eventInfo);

    evas_object_smart_callback_add(frame, "icon,changed", onFrameIconChanged, 0);
    evas_object_smart_callback_add(frame, "intent,new", onFrameIntentNew, 0);
    evas_object_smart_callback_add(frame, "load,provisional", onFrameProvisionalLoad, 0);
    evas_object_smart_callback_add(frame, "load,provisional,failed", onFrameProvisionalLoadFailed, 0);
    evas_object_smart_callback_add(frame, "load,committed", onFrameLoadCommitted, 0);
    evas_object_smart_callback_add(frame, "load,finished", onFrameLoadFinished, 0);
    evas_object_smart_callback_add(frame, "load,error", onFrameLoadError, 0);
    evas_object_smart_callback_add(frame, "redirect,cancelled", onFrameRedirectCancelled, 0);
    evas_object_smart_callback_add(frame, "redirect,load,provisional", onFrameRedirectForProvisionalLoad, 0);
    evas_object_smart_callback_add(frame, "redirect,requested", onFrameRedirectRequested, 0);
    evas_object_smart_callback_add(frame, "title,changed", onFrameTitleChanged, 0);
    evas_object_smart_callback_add(frame, "xss,detected", onDidDetectXSS, 0);
}

void DumpRenderTreeChrome::onFrameProvisionalLoad(void*, Evas_Object* frame, void*)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks()) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - didStartProvisionalLoadForFrame\n", frameName.utf8().data());
    }
}

void DumpRenderTreeChrome::onFrameProvisionalLoadFailed(void*, Evas_Object* frame, void*)
{
    m_provisionalLoadFailedFrame = frame;

    if (!done && gLayoutTestController->dumpFrameLoadCallbacks()) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - didFailProvisionalLoadWithError\n", frameName.utf8().data());
    }
}

void DumpRenderTreeChrome::onFrameLoadCommitted(void*, Evas_Object* frame, void*)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks()) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - didCommitLoadForFrame\n", frameName.utf8().data());
    }
}

void DumpRenderTreeChrome::onFrameLoadFinished(void*, Evas_Object* frame, void* eventInfo)
{
    const Ewk_Frame_Load_Error* error = static_cast<Ewk_Frame_Load_Error*>(eventInfo);

    // EFL port emits both "load,finished" and "load,error" signals in error case.
    // Error case is therefore already handled in onFrameLoadError() and we don't need
    // to handle it here.
    if (error)
        return;

    if (!done && gLayoutTestController->dumpProgressFinishedCallback())
        printf("postProgressFinishedNotification\n");

    if (!done && gLayoutTestController->dumpFrameLoadCallbacks()) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - didFinishLoadForFrame\n", frameName.utf8().data());
    }

    if (frame == topLoadingFrame)
        topLoadingFrameLoadFinished();
}

void DumpRenderTreeChrome::onFrameLoadError(void*, Evas_Object* frame, void*)
{
    // In case of provisional load error, we receive both "load,error" and "load,provisional,failed"
    // signals. m_provisionalLoadFailedFrame is used to avoid printing twice the load error: in
    // onFrameProvisionalLoadFailed() and onFrameLoadError().
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks() && frame != m_provisionalLoadFailedFrame) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - didFailLoadWithError\n", frameName.utf8().data());
    }

    if (m_provisionalLoadFailedFrame && frame == m_provisionalLoadFailedFrame)
        m_provisionalLoadFailedFrame = 0;

    if (frame == topLoadingFrame)
        topLoadingFrameLoadFinished();
}

void DumpRenderTreeChrome::onFrameRedirectCancelled(void*, Evas_Object* frame, void*)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks()) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - didCancelClientRedirectForFrame\n", frameName.utf8().data());
    }
}

void DumpRenderTreeChrome::onFrameRedirectForProvisionalLoad(void*, Evas_Object* frame, void*)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks()) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - didReceiveServerRedirectForProvisionalLoadForFrame\n", frameName.utf8().data());
    }
}

void DumpRenderTreeChrome::onFrameRedirectRequested(void*, Evas_Object* frame, void* eventInfo)
{
    const char* url = static_cast<const char*>(eventInfo);

    if (!done && gLayoutTestController->dumpFrameLoadCallbacks()) {
        const String frameName(DumpRenderTreeSupportEfl::suitableDRTFrameName(frame));
        printf("%s - willPerformClientRedirectToURL: %s \n", frameName.utf8().data(), pathSuitableForTestResult(url).data());
    }
}

void DumpRenderTreeChrome::onDidDetectXSS(void*, Evas_Object* view, void*)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks())
        printf("didDetectXSS\n");
}

void DumpRenderTreeChrome::onResponseReceived(void*, Evas_Object*, void* eventInfo)
{
    Ewk_Frame_Resource_Response* response = static_cast<Ewk_Frame_Resource_Response*>(eventInfo);

    if (!done && gLayoutTestController->dumpResourceLoadCallbacks()) {
        CString responseDescription(descriptionSuitableForTestResult(response));
        printf("%s - didReceiveResponse %s\n",
               m_dumpAssignedUrls.contains(response->identifier) ? m_dumpAssignedUrls.get(response->identifier).data() : "<unknown>",
               responseDescription.data());
    }

    if (!done && gLayoutTestController->dumpResourceResponseMIMETypes()) {
        printf("%s has MIME type %s\n",
               KURL(ParsedURLString, response->url).lastPathComponent().utf8().data(),
               response->mime_type);
    }
}

void DumpRenderTreeChrome::onResourceLoadFinished(void*, Evas_Object*, void* eventInfo)
{
    unsigned long identifier = *static_cast<unsigned long*>(eventInfo);

    if (!done && gLayoutTestController->dumpResourceLoadCallbacks())
        printf("%s - didFinishLoading\n",
               (m_dumpAssignedUrls.contains(identifier) ? m_dumpAssignedUrls.take(identifier).data() : "<unknown>"));
}

void DumpRenderTreeChrome::onResourceLoadFailed(void*, Evas_Object*, void* eventInfo)
{
    Ewk_Frame_Load_Error* error = static_cast<Ewk_Frame_Load_Error*>(eventInfo);

    if (!done && gLayoutTestController->dumpResourceLoadCallbacks())
        printf("%s - didFailLoadingWithError: %s\n",
               (m_dumpAssignedUrls.contains(error->resource_identifier) ? m_dumpAssignedUrls.take(error->resource_identifier).data() : "<unknown>"),
               descriptionSuitableForTestResult(error).data());
}

void DumpRenderTreeChrome::onNewResourceRequest(void*, Evas_Object*, void* eventInfo)
{
    Ewk_Frame_Resource_Request* request = static_cast<Ewk_Frame_Resource_Request*>(eventInfo);

    if (!done && gLayoutTestController->dumpResourceLoadCallbacks())
        m_dumpAssignedUrls.add(request->identifier, pathSuitableForTestResult(request->url));
}

void DumpRenderTreeChrome::onFrameIntentNew(void*, Evas_Object*, void* eventInfo)
{
    Ewk_Intent_Request* request = static_cast<Ewk_Intent_Request*>(eventInfo);
    Ewk_Intent* intent = ewk_intent_request_intent_get(request);
    if (!intent)
        return;

    ewk_intent_request_ref(request);
    if (m_currentIntentRequest)
        ewk_intent_request_unref(m_currentIntentRequest);
    m_currentIntentRequest = request;

    printf("Received Web Intent: action=%s type=%s\n",
           ewk_intent_action_get(intent),
           ewk_intent_type_get(intent));

    // TODO: Display number of ports once Ewk_Intent exposes this information.

    const char* service = ewk_intent_service_get(intent);
    if (service && strcmp(service, ""))
        printf("Explicit intent service: %s\n", service);

    void* data = 0;
    Eina_List* extraNames = ewk_intent_extra_names_get(intent);
    EINA_LIST_FREE(extraNames, data) {
        char* name = static_cast<char*>(data);
        char* value = ewk_intent_extra_get(intent, name);
        if (value) {
            printf("Extras[%s] = %s\n", name, value);
            free(value);
        }
        free(name);
    }

    Eina_List* suggestions = ewk_intent_suggestions_get(intent);
    EINA_LIST_FREE(suggestions, data) {
        printf("Have suggestion %s\n", static_cast<char*>(data));
        free(data);
    }
}
