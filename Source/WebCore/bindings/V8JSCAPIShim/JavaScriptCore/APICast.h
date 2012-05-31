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

#ifndef APICast_h
#define APICast_h

#include "JSBase.h"
#include <v8.h>

namespace V8JSCAPIShim {

// JSValueRef, JSObjectRef

inline v8::Handle<v8::Object> toV8(JSObjectRef wrapper)
{
    return wrapper->get();
}

inline v8::Handle<v8::Value> toV8(JSValueRef wrapper)
{
    return wrapper->get();
}

inline PassRefPtr<ObjectWrapper> toRef(v8::Handle<v8::Object> object)
{
    return adoptRef(ObjectWrapper::create(object).leakRef());
}

inline PassRefPtr<ValueWrapper> toRef(v8::Handle<v8::Value> value)
{
    return adoptRef(ValueWrapper::create(value).leakRef());
}

static inline PassRefPtr<ValueWrapper> jsUndefined()
{
    return toRef(v8::Handle<v8::Value>(v8::Undefined()));
}

static inline PassRefPtr<ValueWrapper> nullValue()
{
    return ValueWrapper::create();
}

static inline PassRefPtr<ObjectWrapper> nullObject()
{
    return ObjectWrapper::create();
}

// ---------------------

// JSPropertyNameArrayRef

typedef Wrapper<v8::Array> ArrayWrapper;

inline ArrayWrapper* toImpl(JSPropertyNameArrayRef ref)
{
    return reinterpret_cast<ArrayWrapper*>(ref);
}

inline v8::Handle<v8::Array> toV8(JSPropertyNameArrayRef ref)
{
    return toImpl(ref)->get();
}

inline JSPropertyNameArrayRef toRef(v8::Handle<v8::Array> array)
{
    return reinterpret_cast<JSPropertyNameArrayRef>(ArrayWrapper::create(array).leakRef());
}

inline JSPropertyNameArrayRef toRef(ArrayWrapper* wrapper)
{
    return reinterpret_cast<JSPropertyNameArrayRef>(wrapper);
}

inline JSPropertyNameArrayRef toRef(PassRefPtr<ArrayWrapper> wrapper)
{
    return toRef(wrapper.get());
}

// ---------------------

// JSContextRef, JSGlobalContextRef

typedef Wrapper<v8::Context> ContextWrapper;

inline ContextWrapper* toImpl(JSContextRef ref)
{
    return const_cast<ContextWrapper*>(reinterpret_cast<const ContextWrapper*>(ref));
}

inline ContextWrapper* toImpl(JSGlobalContextRef ref)
{
    return reinterpret_cast<ContextWrapper*>(ref);
}

inline v8::Handle<v8::Context> toV8(JSContextRef ref)
{
    return toImpl(ref)->get();
}

inline v8::Handle<v8::Context> toV8(JSGlobalContextRef ref)
{
    return toImpl(ref)->get();
}

inline JSContextRef toRef(ContextWrapper* ptr)
{
    return reinterpret_cast<JSContextRef>(ptr);
}

inline JSContextRef toRef(const RefPtr<ContextWrapper > ptr)
{
    return toRef(ptr.get());
}

inline JSGlobalContextRef toGlobalRef(ContextWrapper* ptr)
{
    return reinterpret_cast<JSGlobalContextRef>(ptr);
}

inline JSGlobalContextRef toGlobalRef(PassRefPtr<ContextWrapper > ptr)
{
    return toGlobalRef(ptr.get());
}

inline JSGlobalContextRef toGlobalRef(v8::Handle<v8::Context> handle)
{
    return reinterpret_cast<JSGlobalContextRef>(ContextWrapper::create(handle).leakRef());
}

inline JSContextRef toRef(v8::Handle<v8::Context> handle)
{
    return reinterpret_cast<JSContextRef>(ContextWrapper::create(handle).leakRef());
}

// ---------------------

} // namespace V8JSCAPIShim

using namespace V8JSCAPIShim;

#endif
