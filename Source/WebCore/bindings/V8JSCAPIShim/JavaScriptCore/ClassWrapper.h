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

#ifndef ClassWrapper_h
#define ClassWrapper_h

#include <JavaScriptCore/API/JSObjectRef.h>
#include <v8.h>
#include <wtf/RefPtr.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/Vector.h>

namespace V8JSCAPIShim {

class ClassWrapper : public ThreadSafeRefCounted<ClassWrapper> {
public:
    static PassRefPtr<ClassWrapper> create(const JSClassDefinition* def)
    {
        return adoptRef(new ClassWrapper(def));
    }

    ClassWrapper* parent() const { return m_parent.get(); }
    void setParent(ClassWrapper* parent) { m_parent = parent; }

    v8::Handle<v8::FunctionTemplate> functionTemplate() const { return m_functionT; }
    void setFunctionTemplate(v8::Handle<v8::FunctionTemplate> templ)
    {
        m_functionT = v8::Persistent<v8::FunctionTemplate>::New(templ);
    }

    void takeStaticFuncPtr(v8::Persistent<v8::Value> ptr) { m_staticFunctions.append(ptr); }
    void takeStaticValueCallback(v8::Persistent<v8::Value> ptr) { m_staticValueCallbacksList.append(ptr); }
    void takePropertyCallback(v8::Persistent<v8::Value> ptr) { m_propertyCallbacksList.append(ptr); }

    const JSClassDefinition& classDef() const { return m_definition; }
private:
    friend class ThreadSafeRefCounted<ClassWrapper>;

    explicit ClassWrapper(const JSClassDefinition* definition)
    {
        ASSERT(definition);
        m_definition = *definition;
    }

    ~ClassWrapper();

    v8::Persistent<v8::FunctionTemplate> m_functionT;
    RefPtr<ClassWrapper> m_parent;
    JSClassDefinition m_definition;
    Vector<v8::Persistent<v8::Value> > m_staticFunctions;
    Vector<v8::Persistent<v8::Value> > m_staticValueCallbacksList;
    Vector<v8::Persistent<v8::Value> > m_propertyCallbacksList;
};

struct StaticValueCallbacks {
    bool isEmpty() const
    {
        return !(getProp || setProp);
    }
    JSObjectGetPropertyCallback getProp;
    JSObjectSetPropertyCallback setProp;
};

struct NamedPropertyCallbacks {
    bool isEmpty() const
    {
        return !(getProp || setProp || deleteProp || getPropNames);
    }
    JSObjectGetPropertyCallback getProp;
    JSObjectSetPropertyCallback setProp;
    JSObjectDeletePropertyCallback deleteProp;
    JSObjectGetPropertyNamesCallback getPropNames;
};

inline ClassWrapper* toImpl(JSClassRef ref)
{
    return reinterpret_cast<ClassWrapper*>(ref);
}

inline JSClassRef toRef(ClassWrapper* wrapper)
{
    return reinterpret_cast<JSClassRef>(wrapper);
}

ClassWrapper* getClassOfObject(v8::Handle<v8::Object>);

} // namespace V8JSCAPIShim

#endif
