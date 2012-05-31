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
#include "JSContextRef.h"

#include "APICast.h"
#include "ClassWrapper.h"
#include "JSBase.h"
#include "V8JSCAPIShimMacros.h"
#include <wtf/HashMap.h>
#include <wtf/Threading.h>

using namespace v8;
using namespace V8JSCAPIShim;

class IsolateWrapper : public RefCounted<IsolateWrapper> {
public:
    static PassRefPtr<IsolateWrapper> create(v8::Isolate* isolate)
    {
        return adoptRef(new IsolateWrapper(isolate));
    }
    v8::Isolate* get() const
    {
        return m_isolate;
    }
private:
    friend class RefCounted<IsolateWrapper>;

    explicit IsolateWrapper(v8::Isolate* isolate) : m_isolate(isolate) { }

    ~IsolateWrapper()
    {
        if (v8::Isolate::GetCurrent() == m_isolate)
            m_isolate->Exit();
        m_isolate->Dispose();
    }

    v8::Isolate* m_isolate;
};

inline IsolateWrapper* toImpl(JSContextGroupRef ref)
{
    return reinterpret_cast<IsolateWrapper*>(const_cast<OpaqueJSContextGroup*>(ref));
}

inline JSContextGroupRef toRef(v8::Isolate* isolate)
{
    return reinterpret_cast<JSContextGroupRef>(IsolateWrapper::create(isolate).leakRef());
}

JSContextGroupRef JSContextGroupCreate()
{
    WTF::initializeThreading();

    Isolate* isolate = Isolate::New();
    isolate->Enter();
    return toRef(isolate);
}

JSContextGroupRef JSContextGroupRetain(JSContextGroupRef group)
{
    toImpl(group)->ref();
    return group;
}

void JSContextGroupRelease(JSContextGroupRef group)
{
    toImpl(group)->deref();
}

JSGlobalContextRef JSGlobalContextCreate(JSClassRef globalObjectClass)
{
    WTF::initializeThreading();

    // We rely on v8 to implicitly enter to the context.

    HandleScope scope;
    if (!globalObjectClass)
        return toGlobalRef(Context::New());
    ClassWrapper* globalClass = toImpl(globalObjectClass);
    Local<ObjectTemplate> globalT = globalClass->functionTemplate()->InstanceTemplate();
    Persistent<Context> newContext = Context::New(0, globalT);
    newContext.MakeWeak(0, &weakReferenceDestroyerCallback);
    return toGlobalRef(newContext);
}

JSGlobalContextRef JSGlobalContextCreateInGroup(JSContextGroupRef group, JSClassRef globalObjectClass)
{
    return JSGlobalContextCreate(globalObjectClass);
}

JSGlobalContextRef JSGlobalContextRetain(JSGlobalContextRef context)
{
    toImpl(context)->ref();
    return context;
}

void JSGlobalContextRelease(JSGlobalContextRef context)
{
    toImpl(context)->deref();
}

JSObjectRef JSContextGetGlobalObject(JSContextRef context)
{
    API_PROLOGUE(context);
    return toRef(toV8(context)->Global());
}
