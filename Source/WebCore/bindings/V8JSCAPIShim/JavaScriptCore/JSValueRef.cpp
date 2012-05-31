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
#include "JSValueRef.h"

#include "APICast.h"
#include "ClassWrapper.h"
#include "JSStringRef.h"
#include "OpaqueJSString.h"
#include "V8JSCAPIShimMacros.h"
#include <wtf/HashSet.h>

using namespace v8;
using namespace V8JSCAPIShim;

JSType JSValueGetType(JSContextRef context, JSValueRef value)
{
    API_PROLOGUE(context);
    Handle<Value> v8Value = toV8(value);

    if (v8Value->IsUndefined())
        return kJSTypeUndefined;
    if (v8Value->IsNull())
        return kJSTypeNull;
    if (v8Value->IsBoolean())
        return kJSTypeBoolean;
    if (v8Value->IsNumber())
        return kJSTypeNumber;
    if (v8Value->IsString())
        return kJSTypeString;
    ASSERT(v8Value->IsObject());
    return kJSTypeObject;
}

bool JSValueIsUndefined(JSContextRef context, JSValueRef value)
{
    API_PROLOGUE(context);
    return toV8(value)->IsUndefined();
}

bool JSValueIsNull(JSContextRef context, JSValueRef value)
{
    API_PROLOGUE(context);
    return toV8(value)->IsNull();
}

bool JSValueIsBoolean(JSContextRef context, JSValueRef value)
{
    API_PROLOGUE(context);
    return toV8(value)->IsBoolean();
}

bool JSValueIsNumber(JSContextRef context, JSValueRef value)
{
    API_PROLOGUE(context);
    return toV8(value)->IsNumber();
}

bool JSValueIsString(JSContextRef context, JSValueRef value)
{
    API_PROLOGUE(context);
    return toV8(value)->IsString();
}

bool JSValueIsObject(JSContextRef context, JSValueRef value)
{
    API_PROLOGUE(context);
    return toV8(value)->IsObject();
}

bool JSValueIsObjectOfClass(JSContextRef context, JSValueRef value, JSClassRef jsClass)
{
    API_PROLOGUE(context);
    Handle<Value> jsValue = toV8(value);
    if (!jsValue->IsObject())
        return false;
    Handle<Object> object = jsValue->ToObject();
    ClassWrapper* objClass = V8JSCAPIShim::getClassOfObject(object);
    return objClass == toImpl(jsClass);
}

bool JSValueIsEqual(JSContextRef context, JSValueRef a, JSValueRef b, JSValueRef* exception)
{
    API_PROLOGUE(context);
    Handle<Value> v8A = toV8(a);
    Handle<Value> v8B = toV8(b);
    TryCatch tryCatch;
    bool result = v8A->Equals(v8B);
    ESCAPE_ON_EXCEPTION(tryCatch, exception, false);
    return result;
}

bool JSValueIsStrictEqual(JSContextRef context, JSValueRef a, JSValueRef b)
{
    API_PROLOGUE(context);
    Handle<Value> v8A = toV8(a);
    Handle<Value> v8B = toV8(b);
    return v8A->StrictEquals(v8B);
}

bool JSValueIsInstanceOfConstructor(JSContextRef context, JSValueRef value, JSObjectRef constructor, JSValueRef* exception)
{
    API_PROLOGUE(context);
    Handle<Value> v8Value = toV8(value);
    if (!v8Value->IsObject())
        return false;
    Handle<Object> v8Constructor = toV8(constructor);
    if (!v8Constructor->IsFunction())
        return false;
    return v8Constructor->GetPrototype() == v8Value.As<Object>()->GetPrototype();
}

JSValueRef JSValueMakeUndefined(JSContextRef context)
{
    API_PROLOGUE(context);
    return jsUndefined();
}

JSValueRef JSValueMakeNull(JSContextRef context)
{
    API_PROLOGUE(context);
    return toRef(Handle<Value>(Null()));
}

JSValueRef JSValueMakeBoolean(JSContextRef context, bool value)
{
    API_PROLOGUE(context);
    return toRef(Handle<Value>(Boolean::New(value)));
}

JSValueRef JSValueMakeNumber(JSContextRef context, double value)
{
    API_PROLOGUE(context);
    return toRef(Handle<Value>(Number::New(value)));
}

JSValueRef JSValueMakeString(JSContextRef context, JSStringRef string)
{
    API_PROLOGUE(context);
    return toRef(Handle<Value>(string->toV8String()));
}

bool JSValueToBoolean(JSContextRef context, JSValueRef value)
{
    API_PROLOGUE(context);
    Handle<Value> v8Value = toV8(value);
    return v8Value->BooleanValue();
}

double JSValueToNumber(JSContextRef context, JSValueRef value, JSValueRef* exception)
{
    API_PROLOGUE(context);
    Handle<Value> v8Value = toV8(value);
    TryCatch tryCatch;
    double number = v8Value->NumberValue();
    ESCAPE_ON_EXCEPTION(tryCatch, exception, std::numeric_limits<double>::quiet_NaN());
    return number;
}

JSStringRef JSValueToStringCopy(JSContextRef context, JSValueRef value, JSValueRef *exception)
{
    API_PROLOGUE(context);
    Handle<Value> v8Value = toV8(value);
    TryCatch tryCatch;
    Handle<v8::String> result = v8Value->ToString();
    ESCAPE_ON_EXCEPTION(tryCatch, exception, 0);
    return OpaqueJSString::create(result).leakRef();
}

JSObjectRef JSValueToObject(JSContextRef context, JSValueRef value, JSValueRef* exception)
{
    API_PROLOGUE(context);
    Handle<Value> v8Value = toV8(value);
    TryCatch tryCatch;
    Handle<Object> v8Object = v8Value->ToObject();
    ESCAPE_ON_EXCEPTION(tryCatch, exception, nullObject());
    return toRef(v8Object);
}

// Note: probably protect / unprotect could be noop.

static HashSet<ValueWrapper*> protectedValues()
{
    DEFINE_STATIC_LOCAL(HashSet<ValueWrapper*>, map, ());
    return map;
}

void JSValueProtect(JSContextRef context, JSValueRef value)
{
    if (!value)
        return;
    value->ref();
    protectedValues().add(value.get());
}

void JSValueUnprotect(JSContextRef context, JSValueRef value)
{
    if (!value)
        return;
    HashSet<ValueWrapper*>::iterator found = protectedValues().find(value.get());
    if (found == protectedValues().end())
        return;
    ValueWrapper* ptr = *found.get();
    ptr->deref();
    protectedValues().remove(found);
}
