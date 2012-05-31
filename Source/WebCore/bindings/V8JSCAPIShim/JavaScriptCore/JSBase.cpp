/*
 * Copyright (C) 2012 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2012 University of Szeged. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSBase.h"

#include "APICast.h"
#include "OpaqueJSString.h"
#include "V8JSCAPIShimMacros.h"

using namespace v8;
using namespace V8JSCAPIShim;

JSValueRef JSEvaluateScript(JSContextRef context, JSStringRef script, JSObjectRef thisObject,
                            JSStringRef sourceURL, int startingLineNumber, JSValueRef* exception)
{
    API_PROLOGUE(context);

    Handle<v8::String> url = sourceURL->toV8String();
    TryCatch tryCatch;
    ScriptOrigin origin(url, Integer::New(startingLineNumber), Integer::New(0));
    Handle<Script> src = Script::New(script->toV8String(), &origin);
    ESCAPE_ON_EXCEPTION(tryCatch, exception, nullValue());

    Handle<Value> result = src->Run();
    ESCAPE_ON_EXCEPTION(tryCatch, exception, nullValue());
    return toRef(result);
}

bool JSCheckScriptSyntax(JSContextRef context, JSStringRef script, JSStringRef sourceURL, int startingLineNumber, JSValueRef* exception)
{
    API_PROLOGUE(context);

    Handle<v8::String> url = sourceURL->toV8String();
    TryCatch tryCatch;
    ScriptOrigin origin(url, Integer::New(startingLineNumber), Integer::New(0));
    Script::New(script->toV8String(), &origin);
    ESCAPE_ON_EXCEPTION(tryCatch, exception, false);
    return true;
}

void JSGarbageCollect(JSContextRef context)
{
    V8::LowMemoryNotification();
}
