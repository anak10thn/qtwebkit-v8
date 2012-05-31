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

#ifndef JSBase_h
#define JSBase_h

// -----------------------------------------------------------------
// BEGIN Macros and typedefs coming from the original JSBase.h

/* JavaScript symbol exports */
/* These rules should stay the same as in WebKit2/Shared/API/c/WKBase.h */

#undef JS_EXPORT

#if defined(JS_NO_EXPORT)
#define JS_EXPORT
#elif defined(__GNUC__) && !defined(__CC_ARM) && !defined(__ARMCC__)
#define JS_EXPORT __attribute__((visibility("default")))
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN32_WCE) || defined(__CC_ARM) || defined(__ARMCC__)
#if defined(BUILDING_JavaScriptCore) || defined(BUILDING_WTF)
#define JS_EXPORT __declspec(dllexport)
#else
#define JS_EXPORT __declspec(dllimport)
#endif
#else /* !defined(JS_NO_EXPORT) */
#define JS_EXPORT
#endif /* defined(JS_NO_EXPORT) */

/* JS tests uses WTF but has no config.h, so we need to set the export defines here. */
#ifndef WTF_EXPORT_PRIVATE
#define WTF_EXPORT_PRIVATE JS_EXPORT
#endif

/* JavaScript engine interface */

typedef const struct OpaqueJSContextGroup* JSContextGroupRef;
typedef const struct OpaqueJSContext* JSContextRef;
typedef struct OpaqueJSContext* JSGlobalContextRef;
typedef struct OpaqueJSString* JSStringRef;
typedef struct OpaqueJSClass* JSClassRef;
typedef struct OpaqueJSPropertyNameArray* JSPropertyNameArrayRef;
typedef struct OpaqueJSPropertyNameAccumulator* JSPropertyNameAccumulatorRef;

// END
// -----------------------------------------------------------------

#include <v8.h>
#include <wtf/HashFunctions.h>
#include <wtf/HashTraits.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>

namespace V8JSCAPIShim {

void weakReferenceDestroyerCallback(v8::Persistent<v8::Value>, void*);

// Refcounted wrapper that holds a Persistent.
template <typename T>
class Wrapper : public RefCounted<Wrapper<T> > {
public:
    static PassRefPtr<Wrapper<T> > create(v8::Handle<T> value)
    {
        return adoptRef(new Wrapper<T>(value));
    }
    static PassRefPtr<Wrapper<T> > create()
    {
        return adoptRef(new Wrapper<T>());
    }

    v8::Handle<T> get() const { return m_value; }
    bool isEmpty() const { return m_value.IsEmpty(); }

private:
    friend class RefCounted<Wrapper<T> >;

    Wrapper() { }
    explicit Wrapper(v8::Handle<T> value);

    ~Wrapper()
    {
        if (m_value.IsEmpty())
           return;
        m_value.Dispose();
        m_value.Clear();
    }

    v8::Persistent<T> m_value;
};

template <typename T>
inline Wrapper<T>::Wrapper(v8::Handle<T> value)
{
    v8::HandleScope scope;
    ASSERT(!value.IsEmpty());
    m_value = v8::Persistent<T>::New(value);
}

typedef Wrapper<v8::Object> ObjectWrapper;
typedef Wrapper<v8::Value> ValueWrapper;

// We use smart pointers to fill the gap between the implicit
// memory handling in JavaScriptCore and the Handle concept of v8.
// We need to constumize RefPtr to be possible to cast from
// JSObjectRef to JSValueRef and to assign null to each.

class ObjectWrapperRefPtr : public RefPtr<ObjectWrapper> {
    typedef RefPtr<ObjectWrapper> Base;
    typedef PassRefPtr<ObjectWrapper> PassBase;
public:
    ObjectWrapperRefPtr()
        : Base()
    { }
    ObjectWrapperRefPtr(const PassBase& ptr)
        : Base(ptr)
    { }
    ObjectWrapperRefPtr& operator =(const PassBase& ptr)
    {
        *static_cast<Base*>(this) = ptr;
        return *this;
    }
    ObjectWrapperRefPtr(void* ptr)
        : Base()
    {
        COMPILE_ASSERT(!ptr, only_null_is_allowed_to_be_passed_as_raw_pointer);
    }
    ObjectWrapperRefPtr& operator =(void* ptr)
    {
        *this = ObjectWrapperRefPtr();
        COMPILE_ASSERT(!ptr, only_null_is_allowed_to_be_passed_as_raw_pointer);
        return *this;
    }
};

class ValueWrapperRefPtr : public RefPtr<ValueWrapper> {
    typedef RefPtr<ValueWrapper> Base;
    typedef PassRefPtr<ValueWrapper> PassBase;
public:
    ValueWrapperRefPtr()
        : Base()
    { }
    ValueWrapperRefPtr(const PassBase& ptr)
        : Base(ptr)
    { }
    ValueWrapperRefPtr& operator =(const PassBase& ptr)
    {
        *static_cast<RefPtr<ValueWrapper>*>(this) = ptr;
        return *this;
    }
    ValueWrapperRefPtr(void* ptr)
        : Base()
    {
        COMPILE_ASSERT(!ptr, should_only_pass_null_as_raw_pointer);
    }
    ValueWrapperRefPtr& operator =(void* ptr)
    {
        *this = ValueWrapperRefPtr();
        COMPILE_ASSERT(!ptr, only_null_is_allowed_to_be_passed_as_raw_pointer);
        return *this;
    }
    ValueWrapperRefPtr(const ObjectWrapperRefPtr& ptr)
        : Base(ValueWrapper::create(v8::Handle<v8::Value>(ptr->get())))
    { }
    ValueWrapperRefPtr(const PassRefPtr<ObjectWrapper>& ptr)
        : Base(ValueWrapper::create(v8::Handle<v8::Value>(ptr.leakRef()->get())))
    { }

    // C style cast: (JSObjectRef)value
    operator PassRefPtr<ObjectWrapper> ()
    {
        ASSERT(get()->get()->IsObject());
        return ObjectWrapper::create(get()->get().As<v8::Object>());
    }
};

} // namescpace V8JSCAPIShim

typedef V8JSCAPIShim::ValueWrapperRefPtr JSValueRef;
typedef V8JSCAPIShim::ObjectWrapperRefPtr JSObjectRef;

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------
// BEGIN API coming from the original JSBase.h

/* Script Evaluation */

/*!
@function JSEvaluateScript
@abstract Evaluates a string of JavaScript.
@param ctx The execution context to use.
@param script A JSString containing the script to evaluate.
@param thisObject The object to use as "this," or NULL to use the global object as "this."
@param sourceURL A JSString containing a URL for the script's source file. This is only used when reporting exceptions. Pass NULL if you do not care to include source file information in exceptions.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The JSValue that results from evaluating script, or NULL if an exception is thrown.
*/
JS_EXPORT JSValueRef JSEvaluateScript(JSContextRef ctx, JSStringRef script, JSObjectRef thisObject, JSStringRef sourceURL, int startingLineNumber, JSValueRef* exception);

/*!
@function JSCheckScriptSyntax
@abstract Checks for syntax errors in a string of JavaScript.
@param ctx The execution context to use.
@param script A JSString containing the script to check for syntax errors.
@param sourceURL A JSString containing a URL for the script's source file. This is only used when reporting exceptions. Pass NULL if you do not care to include source file information in exceptions.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions.
@param exception A pointer to a JSValueRef in which to store a syntax error exception, if any. Pass NULL if you do not care to store a syntax error exception.
@result true if the script is syntactically correct, otherwise false.
*/
JS_EXPORT bool JSCheckScriptSyntax(JSContextRef ctx, JSStringRef script, JSStringRef sourceURL, int startingLineNumber, JSValueRef* exception);

/*!
@function JSGarbageCollect
@abstract Performs a JavaScript garbage collection.
@param ctx The execution context to use.
@discussion JavaScript values that are on the machine stack, in a register,
 protected by JSValueProtect, set as the global object of an execution context,
 or reachable from any such value will not be collected.

 During JavaScript execution, you are not required to call this function; the
 JavaScript engine will garbage collect as needed. JavaScript values created
 within a context group are automatically destroyed when the last reference
 to the context group is released.
*/
JS_EXPORT void JSGarbageCollect(JSContextRef ctx);

#ifdef __cplusplus
}
#endif

// END
// -----------------------------------------------------------------

#endif /* JSBase_h */
