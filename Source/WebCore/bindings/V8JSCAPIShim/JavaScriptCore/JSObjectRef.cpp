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
#include "JSObjectRef.h"

#include "APICast.h"
#include "ClassWrapper.h"
#include "OpaqueJSString.h"
#include "V8JSCAPIShimMacros.h"
#include <wtf/HashSet.h>
#include <wtf/Vector.h>

using namespace v8;

const JSClassDefinition kJSClassDefinitionEmpty = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

namespace V8JSCAPIShim {

void weakReferenceDestroyerCallback(Persistent<Value> p, void*)
{
    p.Dispose();
    p.Clear();
}

static Handle<Value> staticFunctionCallThunk(const Arguments& args)
{
    HandleScope handleScope;
    Handle<Value> data = args.Data();
    JSObjectCallAsFunctionCallback funcPtr = bitwise_cast<JSObjectCallAsFunctionCallback>(External::Unwrap(data));
    uint nArgs = args.Length();
    Vector<JSValueRef> argsW(nArgs);
    for (uint i = 0; i < nArgs; ++i)
        argsW[i] = toRef(args[i]);

    Handle<Context> context = Context::GetCurrent();
    RefPtr<ContextWrapper> contextW = ContextWrapper::create(context);

    COMPILE_ASSERT(sizeof(JSValueRef) == sizeof(void*), JSValueRef_must_be_pointer_sized);
    const JSValueRef* argsAsAPI = reinterpret_cast<const JSValueRef*>(argsW.data());

    JSValueRef exception;
    JSValueRef result(funcPtr(toRef(contextW),
                              toRef(args.Callee().As<Object>()),
                              toRef(args.This()),
                              nArgs,
                              argsAsAPI,
                              &exception));
    if (exception)
        ThrowException(exception->get());

    return handleScope.Close(result->get());
}

static Handle<Value> staticValueGetterThunk(Local<v8::String> property, const AccessorInfo& info)
{
    HandleScope handleScope;
    StaticValueCallbacks* callbacks = bitwise_cast<StaticValueCallbacks*>(External::Unwrap(info.Data()));
    JSObjectGetPropertyCallback funcPtr = callbacks->getProp;
    RefPtr<ContextWrapper> context = ContextWrapper::create(Context::GetCurrent());
    RefPtr<OpaqueJSString> prop = OpaqueJSString::create(property);
    JSValueRef exception;
    JSValueRef result(funcPtr(toRef(context), toRef(info.This()), prop.get(), &exception));
    if (exception)
        ThrowException(exception->get());
    return result ? handleScope.Close(result->get()) : Handle<Value>();
}

static void staticValueSetterThunk(Local<v8::String> property, Local<Value> value, const AccessorInfo& info)
{
    HandleScope handleScope;
    StaticValueCallbacks* callbacks = bitwise_cast<StaticValueCallbacks*>(External::Unwrap(info.Data()));
    JSObjectSetPropertyCallback funcPtr = callbacks->setProp;
    RefPtr<ContextWrapper> context = ContextWrapper::create(Context::GetCurrent());
    RefPtr<OpaqueJSString> prop = OpaqueJSString::create(property);
    JSValueRef exception;
    funcPtr(toRef(context), toRef(info.This()), prop.get(), toRef(value), &exception);
    if (exception)
        ThrowException(exception->get());
}

static inline PropertyAttribute toV8Attributes(JSPropertyAttributes attribute)
{
    int result = v8::None;
    if (attribute & kJSPropertyAttributeReadOnly)
        result |= v8::ReadOnly;
    if (attribute & kJSPropertyAttributeDontEnum)
        result |= v8::DontEnum;
    if (attribute & kJSPropertyAttributeDontDelete)
        result |= v8::DontDelete;
    return PropertyAttribute(result);
}

static Handle<Value> getPropertyThunk(Local<v8::String> property, const AccessorInfo& info)
{
    HandleScope handleScope;
    NamedPropertyCallbacks* callbacks = bitwise_cast<NamedPropertyCallbacks*>(External::Unwrap(info.Data()));
    JSObjectGetPropertyCallback funcPtr = callbacks->getProp;
    RefPtr<ContextWrapper> context = ContextWrapper::create(Context::GetCurrent());
    RefPtr<OpaqueJSString> prop = OpaqueJSString::create(property);
    JSValueRef exception;
    JSValueRef result(funcPtr(toRef(context), toRef(info.This()), prop.get(), &exception));
    if (exception)
        ThrowException(exception->get());
    return result ? handleScope.Close(result->get()) : Handle<Value>();
}

static Handle<Value> setPropertyThunk(Local<v8::String> property, Local<Value> value, const AccessorInfo& info)
{
    HandleScope handleScope;
    NamedPropertyCallbacks* callbacks = bitwise_cast<NamedPropertyCallbacks*>(External::Unwrap(info.Data()));
    JSObjectSetPropertyCallback funcPtr = callbacks->setProp;
    RefPtr<ContextWrapper> context = ContextWrapper::create(Context::GetCurrent());
    RefPtr<OpaqueJSString> prop = OpaqueJSString::create(property);
    JSValueRef exception;
    bool hasProperty = funcPtr(toRef(context), toRef(info.This()), prop.get(), toRef(value), &exception);
    if (exception)
        ThrowException(exception->get());
    return hasProperty ? handleScope.Close(value) : Handle<Value>();
}

static Handle<Boolean> propertyDeleterThunk(Local<v8::String> property, const AccessorInfo& info)
{
    HandleScope handleScope;
    NamedPropertyCallbacks* callbacks = bitwise_cast<NamedPropertyCallbacks*>(External::Unwrap(info.Data()));
    JSObjectDeletePropertyCallback funcPtr = callbacks->deleteProp;
    RefPtr<ContextWrapper> context = ContextWrapper::create(Context::GetCurrent());
    RefPtr<OpaqueJSString> prop = OpaqueJSString::create(property);
    JSValueRef exception;
    bool hasProperty = funcPtr(toRef(context), toRef(info.This()), prop.get(), &exception);
    if (exception)
        ThrowException(exception->get());
    return Boolean::New(hasProperty);
}

} // namespace V8JSCAPIShim

struct OpaqueJSPropertyNameAccumulator {
    Vector<JSStringRef> data;
};

namespace V8JSCAPIShim {

static Handle<Array> getPropertyNamesThunk(const AccessorInfo& info)
{
    HandleScope handleScope;
    NamedPropertyCallbacks* callbacks = bitwise_cast<NamedPropertyCallbacks*>(External::Unwrap(info.Data()));
    JSObjectGetPropertyNamesCallback funcPtr = callbacks->getPropNames;
    RefPtr<ContextWrapper> context = ContextWrapper::create(Context::GetCurrent());
    OpaqueJSPropertyNameAccumulator accumulator;
    funcPtr(toRef(context), toRef(info.This()), &accumulator);
    const Vector<JSStringRef>& data = accumulator.data;
    Local<Array> result = Array::New(data.size());
    for (unsigned i = 0; i < data.size(); ++i) {
        RefPtr<OpaqueJSString> name = adoptRef(data[i]);
        result->Set(i, name->toV8String());
    }
    return handleScope.Close(result);
}

} // namespace V8JSCAPIShim

using namespace V8JSCAPIShim;

JSClassRef JSClassCreate(const JSClassDefinition* definition)
{
    HandleScope handleScope;

    RefPtr<ClassWrapper> wrapper = ClassWrapper::create(definition);
    bool useProto = !(definition->attributes & kJSClassAttributeNoAutomaticPrototype);
    Handle<FunctionTemplate> funcT = FunctionTemplate::New();
    if (definition->parentClass) {
        ClassWrapper* parent = toImpl(definition->parentClass);
        wrapper->setParent(parent);
        funcT->Inherit(parent->functionTemplate());
    }

    if (definition->className)
        funcT->SetClassName(v8::String::New(definition->className));

    Handle<ObjectTemplate> protoT = funcT->PrototypeTemplate();
    Handle<ObjectTemplate> objT = funcT->InstanceTemplate();
    objT->SetInternalFieldCount(1);
    Handle<ObjectTemplate> sFuncsTarget = useProto ? protoT : objT;
    const JSStaticFunction* sFunc = definition->staticFunctions;
    while (sFunc && sFunc->name) {
        ASSERT(sFunc->callAsFunction);
        Persistent<Value> funcPtr = Persistent<Value>::New(External::Wrap(bitwise_cast<void*>(sFunc->callAsFunction)));
        wrapper->takeStaticFuncPtr(funcPtr);
        sFuncsTarget->Set(v8::String::New(sFunc->name), FunctionTemplate::New(staticFunctionCallThunk, funcPtr),
                          toV8Attributes(sFunc->attributes));
        ++sFunc;
    }

    const JSStaticValue* sValue = definition->staticValues;
    while (sValue && sValue->name) {
        StaticValueCallbacks* callbacks = new StaticValueCallbacks;
        callbacks->getProp = sValue->getProperty;
        callbacks->setProp = sValue->setProperty;
        ASSERT(!callbacks->isEmpty());
        Persistent<Value> data = Persistent<Value>::New(External::Wrap(bitwise_cast<void*>(callbacks)));
        wrapper->takeStaticValueCallback(data);
        objT->SetAccessor(v8::String::New(sValue->name), sValue->getProperty ? &staticValueGetterThunk : 0,
                          sValue->setProperty ? &staticValueSetterThunk : 0, data, DEFAULT, toV8Attributes(sValue->attributes));
        ++sValue;
    }

    NamedPropertyCallbacks* propFuncs = new NamedPropertyCallbacks;
    propFuncs->getProp = definition->getProperty;
    propFuncs->setProp = definition->setProperty;
    propFuncs->deleteProp = definition->deleteProperty;
    propFuncs->getPropNames = definition->getPropertyNames;
    if (!propFuncs->isEmpty()) {
        // FIXME: missing mapping for hasProperty.
        if (definition->hasProperty && !definition->getProperty) {
            // If there is a getProperty it can be used instead, otherwise this is fatal.
            FATAL("V8JSCAPIShim: missing implementation for JSClassDefinition::hasProperty");
        }

        Persistent<Value> propertyData = Persistent<Value>::New(External::Wrap(bitwise_cast<void*>(propFuncs)));
        wrapper->takePropertyCallback(propertyData);
        objT->SetNamedPropertyHandler(definition->getProperty ? &getPropertyThunk : 0,
                                      definition->setProperty ? &setPropertyThunk : 0,
                                      0,
                                      definition->deleteProperty ? &propertyDeleterThunk : 0,
                                      definition->getPropertyNames ? &getPropertyNamesThunk : 0,
                                      propertyData);
    }

    if (definition->callAsFunction) {
        Persistent<Value> callAsFuncData = Persistent<Value>::New(External::Wrap(bitwise_cast<void*>(definition->callAsFunction)));
        wrapper->takeStaticFuncPtr(callAsFuncData);
        objT->SetCallAsFunctionHandler(&staticFunctionCallThunk, callAsFuncData);
    }

    // FIXME: missing mapping for callAsConstructor, hasInstance, convertToType.
    if (definition->callAsConstructor)
        FATAL("V8JSCAPIShim: missing implementation for JSClassDefinition::callAsConstructor");
    if (definition->hasInstance)
        FATAL("V8JSCAPIShim: missing implementation for JSClassDefinition::hasInstance");
    if (definition->convertToType)
        FATAL("V8JSCAPIShim: missing implementation for JSClassDefinition::convertToType");

    wrapper->setFunctionTemplate(funcT);
    return toRef(wrapper.release().leakRef());
}

JSClassRef JSClassRetain(JSClassRef jsClass)
{
    toImpl(jsClass)->ref();
    return jsClass;
}

void JSClassRelease(JSClassRef jsClass)
{
    toImpl(jsClass)->deref();
}

namespace V8JSCAPIShim {

struct EmbeddedObjectData;

static HashSet<EmbeddedObjectData*>& allEmbeddedObjectData()
{
    DEFINE_STATIC_LOCAL(HashSet<EmbeddedObjectData*>, map, ());
    return map;
}

struct EmbeddedObjectData {
    EmbeddedObjectData(ClassWrapper* _jsClass, void* data)
        : jsClass(_jsClass)
        , privateData(data)
    {
        allEmbeddedObjectData().add(this);
    }

    static bool isEmbeddedObjectData(void* ptr)
    {
        return allEmbeddedObjectData().contains(static_cast<EmbeddedObjectData*>(ptr));
    }

    ClassWrapper* jsClass;
    void* privateData;
    Persistent<Object> weakRef;
};

void embeddedObjectDestroyerCallback(Persistent<Value> objectAsValue, void*);

}

JSObjectRef JSObjectMake(JSContextRef context, JSClassRef jsClass, void* data)
{
    API_PROLOGUE(context);
    if (!jsClass)
        return toRef(Object::New());

    ClassWrapper* classW = toImpl(jsClass);
    classW->ref();
    Handle<Object> object = classW->functionTemplate()->InstanceTemplate()->NewInstance();
    EmbeddedObjectData* embData = new EmbeddedObjectData(classW, data);
    object->SetPointerInInternalField(0, embData);

    embData->weakRef = Persistent<Object>::New(object);
    embData->weakRef.MakeWeak(0, &embeddedObjectDestroyerCallback);

    // Call initializers from ancestor to descendant.
    Vector<JSObjectInitializeCallback> initializers;
    ClassWrapper* current = classW;
    while (current) {
        const JSClassDefinition& def = current->classDef();
        if (def.initialize)
            initializers.append(def.initialize);
        current = current->parent();
    }
    JSObjectRef objRef = toRef(object);
    for (int i = initializers.size() - 1; i >= 0 ; --i)
        (initializers[i])(context, objRef);

    return objRef;
}

namespace V8JSCAPIShim {

ClassWrapper* getClassOfObject(Handle<Object> object)
{
    if (!object->InternalFieldCount())
        return 0;
    EmbeddedObjectData* data = static_cast<EmbeddedObjectData*>(object->GetPointerFromInternalField(0));
    if (!EmbeddedObjectData::isEmbeddedObjectData(data))
        return 0;
    return data->jsClass;
}

void embeddedObjectDestroyerCallback(Persistent<Value> objectAsValue, void*)
{
    HandleScope handleScope;
    ASSERT(objectAsValue->IsObject());
    Handle<Object> object = objectAsValue.As<Object>();
    ASSERT(object->InternalFieldCount());

    EmbeddedObjectData* data = static_cast<EmbeddedObjectData*>(object->GetPointerFromInternalField(0));
    ASSERT(*data->weakRef == *objectAsValue);

    ClassWrapper* jsClass = data->jsClass;

    // Call finalizers from descendant to ancestor.
    JSObjectRef objAPI(toRef(object));
    ClassWrapper* currentClass = jsClass;
    do {
        if (JSObjectFinalizeCallback finalizer = currentClass->classDef().finalize)
            finalizer(objAPI);
        currentClass = jsClass->parent();
    } while (currentClass);

    jsClass->deref();

    objectAsValue.Dispose();
    objectAsValue.Clear();
    data->weakRef.Clear();
    delete data;
}

} // namespace V8JSCAPIShim

void* JSObjectGetPrivate(JSObjectRef object)
{
    HandleScope handleScope;
    Handle<Object> obj = object->get();
    if (!obj->InternalFieldCount())
        return 0;
    EmbeddedObjectData* data = static_cast<EmbeddedObjectData*>(object->get()->GetPointerFromInternalField(0));
    if (!EmbeddedObjectData::isEmbeddedObjectData(data))
        return 0;
    return data->privateData;
}

bool JSObjectSetPrivate(JSObjectRef object, void* data)
{
    HandleScope handleScope;
    Handle<Object> obj = object->get();
    if (!obj->InternalFieldCount())
        return false;
    EmbeddedObjectData* embData = static_cast<EmbeddedObjectData*>(object->get()->GetPointerFromInternalField(0));
    if (!EmbeddedObjectData::isEmbeddedObjectData(embData))
        return 0;
    embData->privateData = data;
    return true;
}

void JSPropertyNameAccumulatorAddName(JSPropertyNameAccumulatorRef accumulator, JSStringRef propertyName)
{
    HandleScope handleScope;
    // This can only be  called from callbacks called from JS.
    ASSERT(Context::InContext());
    accumulator->data.append(propertyName);
}

JSPropertyNameArrayRef JSObjectCopyPropertyNames(JSContextRef context, JSObjectRef object)
{
    API_PROLOGUE(context);
    Handle<Object> obj = object->get();
    return toRef(obj->GetPropertyNames());
}

JSPropertyNameArrayRef JSPropertyNameArrayRetain(JSPropertyNameArrayRef array)
{
    toImpl(array)->ref();
    return array;
}

void JSPropertyNameArrayRelease(JSPropertyNameArrayRef array)
{
    toImpl(array)->deref();
}

size_t JSPropertyNameArrayGetCount(JSPropertyNameArrayRef array)
{
    return toImpl(array)->get()->Length();
}

JSStringRef JSPropertyNameArrayGetNameAtIndex(JSPropertyNameArrayRef arrayRef, size_t index)
{
    HandleScope handleScope;
    Handle<Array> array = toImpl(arrayRef)->get();
    Handle<v8::String> name = array->Get(index).As<v8::String>();
    return OpaqueJSString::create(name).leakRef();
}

JSObjectRef JSObjectMakeFunctionWithCallback(JSContextRef context, JSStringRef name, JSObjectCallAsFunctionCallback callAsFunction)
{
    API_PROLOGUE(context);
    Persistent<Value> funcPtr = Persistent<Value>::New(External::Wrap(bitwise_cast<void*>(callAsFunction)));
    funcPtr.MakeWeak(0, weakReferenceDestroyerCallback);
    Handle<FunctionTemplate> funcT = FunctionTemplate::New(&staticFunctionCallThunk, funcPtr);
    Handle<v8::Function> funcObj = funcT->GetFunction();
    funcObj->SetName(name->toV8String());
    return toRef(funcObj.As<Object>());
}

JSObjectRef JSObjectMakeArray(JSContextRef context, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    API_PROLOGUE(context);
    TryCatch tryCatch;
    Handle<Array> array = Array::New(argumentCount);
    ESCAPE_ON_EXCEPTION(tryCatch, exception, nullObject());
    for (unsigned i = 0; i < argumentCount; ++i) {
        array->Set(i, arguments[i]->get());
        ESCAPE_ON_EXCEPTION(tryCatch, exception, nullObject());
    }
    return toRef(array.As<Object>());
}

JSValueRef JSObjectGetPrototype(JSContextRef context, JSObjectRef object)
{
    API_PROLOGUE(context);
    return toRef(object->get()->GetPrototype());
}

JSValueRef JSObjectGetProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    API_PROLOGUE(context);
    TryCatch tryCatch;
    Handle<Value> result = object->get()->Get(propertyName->toV8String());
    ESCAPE_ON_EXCEPTION(tryCatch, exception, jsUndefined());
    return toRef(result);
}

void JSObjectSetProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSPropertyAttributes attributes, JSValueRef* exception)
{
    API_PROLOGUE(context);
    TryCatch tryCatch;
    object->get()->Set(propertyName->toV8String(), value->get(), toV8Attributes(attributes));
    ESCAPE_ON_EXCEPTION_VOID(tryCatch, exception);
}

bool JSObjectDeleteProperty(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    API_PROLOGUE(context);
    TryCatch tryCatch;
    bool result = object->get()->Delete(propertyName->toV8String());
    ESCAPE_ON_EXCEPTION(tryCatch, exception, false);
    return result;
}

JSValueRef JSObjectGetPropertyAtIndex(JSContextRef context, JSObjectRef object, unsigned propertyIndex, JSValueRef* exception)
{
    API_PROLOGUE(context);
    TryCatch tryCatch;
    Handle<Value> result = object->get()->Get(propertyIndex);
    ESCAPE_ON_EXCEPTION(tryCatch, exception, jsUndefined());
    return toRef(result);
}

void JSObjectSetPropertyAtIndex(JSContextRef context, JSObjectRef object, unsigned propertyIndex, JSValueRef value, JSValueRef* exception)
{
    API_PROLOGUE(context);
    TryCatch tryCatch;
    object->get()->Set(propertyIndex, value->get());
    ESCAPE_ON_EXCEPTION_VOID(tryCatch, exception);
}

bool JSObjectIsFunction(JSContextRef context, JSObjectRef object)
{
    API_PROLOGUE(context);
    return object->get()->IsFunction();
}

JSValueRef JSObjectCallAsFunction(JSContextRef context, JSObjectRef object, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    API_PROLOGUE(context);
    TryCatch tryCatch;
    Handle<Object> obj = object->get();
    Handle<Object> thisObj = thisObject->get();
    Vector<Handle<Value> > args(argumentCount);
    for (unsigned i = 0; i < argumentCount; ++i)
        args.append(arguments[i]->get());

    if (obj->IsFunction()) {
        Handle<v8::Function> func = obj.As<v8::Function>();
        Handle<Value> result = func->Call(thisObj, args.size(), args.data());
        ESCAPE_ON_EXCEPTION(tryCatch, exception, nullValue());
        return toRef(result);
    }
    if (obj->IsCallable()) {
        Handle<Value> result = obj->CallAsFunction(thisObj, args.size(), args.data());
        ESCAPE_ON_EXCEPTION(tryCatch, exception, nullValue());
        return toRef(result);
    }

    return nullValue();
}
