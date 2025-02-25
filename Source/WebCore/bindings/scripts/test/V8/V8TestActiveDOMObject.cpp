/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "config.h"
#include "V8TestActiveDOMObject.h"

#include "ContextEnabledFeatures.h"
#include "ExceptionCode.h"
#include "RuntimeEnabledFeatures.h"
#include "V8Binding.h"
#include "V8BindingMacros.h"
#include "V8BindingState.h"
#include "V8DOMWrapper.h"
#include "V8IsolatedContext.h"
#include "V8Node.h"
#include "V8Proxy.h"
#include <wtf/UnusedParam.h>

namespace WebCore {

WrapperTypeInfo V8TestActiveDOMObject::info = { V8TestActiveDOMObject::GetTemplate, V8TestActiveDOMObject::derefObject, 0, 0 };

namespace TestActiveDOMObjectV8Internal {

template <typename T> void V8_USE(T) { }

static v8::Handle<v8::Value> excitingAttrAttrGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    INC_STATS("DOM.TestActiveDOMObject.excitingAttr._get");
    TestActiveDOMObject* imp = V8TestActiveDOMObject::toNative(info.Holder());
    return v8::Integer::New(imp->excitingAttr());
}

static v8::Handle<v8::Value> excitingFunctionCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.TestActiveDOMObject.excitingFunction");
    if (args.Length() < 1)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    TestActiveDOMObject* imp = V8TestActiveDOMObject::toNative(args.Holder());
    if (!V8BindingSecurity::canAccessFrame(V8BindingState::Only(), imp->frame(), true))
        return v8::Handle<v8::Value>();
    EXCEPTION_BLOCK(Node*, nextChild, V8Node::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Node::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
    imp->excitingFunction(nextChild);
    return v8::Handle<v8::Value>();
}

static v8::Handle<v8::Value> postMessageCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.TestActiveDOMObject.postMessage");
    if (args.Length() < 1)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    TestActiveDOMObject* imp = V8TestActiveDOMObject::toNative(args.Holder());
    STRING_TO_V8PARAMETER_EXCEPTION_BLOCK(V8Parameter<>, message, MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined));
    imp->postMessage(message);
    return v8::Handle<v8::Value>();
}

static v8::Handle<v8::Value> postMessageAttrGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    INC_STATS("DOM.TestActiveDOMObject.postMessage._get");
    static v8::Persistent<v8::FunctionTemplate> privateTemplate = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(TestActiveDOMObjectV8Internal::postMessageCallback, v8::Handle<v8::Value>(), v8::Signature::New(V8TestActiveDOMObject::GetRawTemplate())));
    v8::Handle<v8::Object> holder = V8DOMWrapper::lookupDOMWrapper(V8TestActiveDOMObject::GetTemplate(), info.This());
    if (holder.IsEmpty()) {
        // can only reach here by 'object.__proto__.func', and it should passed
        // domain security check already
        return privateTemplate->GetFunction();
    }
    TestActiveDOMObject* imp = V8TestActiveDOMObject::toNative(holder);
    if (!V8BindingSecurity::canAccessFrame(V8BindingState::Only(), imp->frame(), false)) {
        static v8::Persistent<v8::FunctionTemplate> sharedTemplate = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(TestActiveDOMObjectV8Internal::postMessageCallback, v8::Handle<v8::Value>(), v8::Signature::New(V8TestActiveDOMObject::GetRawTemplate())));
        return sharedTemplate->GetFunction();
    }
    return privateTemplate->GetFunction();
}

} // namespace TestActiveDOMObjectV8Internal

static const BatchedAttribute TestActiveDOMObjectAttrs[] = {
    // Attribute 'excitingAttr' (Type: 'readonly attribute' ExtAttr: '')
    {"excitingAttr", TestActiveDOMObjectV8Internal::excitingAttrAttrGetter, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
};

static v8::Persistent<v8::FunctionTemplate> ConfigureV8TestActiveDOMObjectTemplate(v8::Persistent<v8::FunctionTemplate> desc)
{
    desc->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = configureTemplate(desc, "TestActiveDOMObject", v8::Persistent<v8::FunctionTemplate>(), V8TestActiveDOMObject::internalFieldCount,
        TestActiveDOMObjectAttrs, WTF_ARRAY_LENGTH(TestActiveDOMObjectAttrs),
        0, 0);
    UNUSED_PARAM(defaultSignature); // In some cases, it will not be used.
    v8::Local<v8::ObjectTemplate> instance = desc->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> proto = desc->PrototypeTemplate();
    UNUSED_PARAM(instance); // In some cases, it will not be used.
    UNUSED_PARAM(proto); // In some cases, it will not be used.
    instance->SetAccessCheckCallbacks(V8TestActiveDOMObject::namedSecurityCheck, V8TestActiveDOMObject::indexedSecurityCheck, v8::External::Wrap(&V8TestActiveDOMObject::info));

    // Custom Signature 'excitingFunction'
    const int excitingFunctionArgc = 1;
    v8::Handle<v8::FunctionTemplate> excitingFunctionArgv[excitingFunctionArgc] = { V8Node::GetRawTemplate() };
    v8::Handle<v8::Signature> excitingFunctionSignature = v8::Signature::New(desc, excitingFunctionArgc, excitingFunctionArgv);
    proto->Set(v8::String::New("excitingFunction"), v8::FunctionTemplate::New(TestActiveDOMObjectV8Internal::excitingFunctionCallback, v8::Handle<v8::Value>(), excitingFunctionSignature));

    // Function 'postMessage' (ExtAttr: 'DoNotCheckSecurity')
    proto->SetAccessor(v8::String::New("postMessage"), TestActiveDOMObjectV8Internal::postMessageAttrGetter, 0, v8::Handle<v8::Value>(), v8::ALL_CAN_READ, static_cast<v8::PropertyAttribute>(v8::DontDelete | v8::ReadOnly));

    // Custom toString template
    desc->Set(getToStringName(), getToStringTemplate());
    return desc;
}

v8::Persistent<v8::FunctionTemplate> V8TestActiveDOMObject::GetRawTemplate()
{
    V8BindingPerIsolateData* data = V8BindingPerIsolateData::current();
    V8BindingPerIsolateData::TemplateMap::iterator result = data->rawTemplateMap().find(&info);
    if (result != data->rawTemplateMap().end())
        return result->second;

    v8::HandleScope handleScope;
    v8::Persistent<v8::FunctionTemplate> templ = createRawTemplate();
    data->rawTemplateMap().add(&info, templ);
    return templ;
}

v8::Persistent<v8::FunctionTemplate> V8TestActiveDOMObject::GetTemplate()
{
    V8BindingPerIsolateData* data = V8BindingPerIsolateData::current();
    V8BindingPerIsolateData::TemplateMap::iterator result = data->templateMap().find(&info);
    if (result != data->templateMap().end())
        return result->second;

    v8::HandleScope handleScope;
    v8::Persistent<v8::FunctionTemplate> templ =
        ConfigureV8TestActiveDOMObjectTemplate(GetRawTemplate());
    data->templateMap().add(&info, templ);
    return templ;
}

bool V8TestActiveDOMObject::HasInstance(v8::Handle<v8::Value> value)
{
    return GetRawTemplate()->HasInstance(value);
}


v8::Handle<v8::Object> V8TestActiveDOMObject::wrapSlow(PassRefPtr<TestActiveDOMObject> impl, v8::Isolate* isolate)
{
    v8::Handle<v8::Object> wrapper;
    V8Proxy* proxy = 0;
    if (impl->frame()) {
        proxy = V8Proxy::retrieve(impl->frame());
        if (proxy)
            proxy->windowShell()->initContextIfNeeded();
    }

    // Enter the node's context and create the wrapper in that context.
    v8::Handle<v8::Context> context;
    if (proxy && !proxy->matchesCurrentContext()) {
        // For performance, we enter the context only if the currently running context
        // is different from the context that we are about to enter.
        context = proxy->context();
        if (!context.IsEmpty())
            context->Enter();
    }
    wrapper = V8DOMWrapper::instantiateV8Object(proxy, &info, impl.get());
    // Exit the node's context if it was entered.
    if (!context.IsEmpty())
        context->Exit();
    if (UNLIKELY(wrapper.IsEmpty()))
        return wrapper;

    v8::Persistent<v8::Object> wrapperHandle = v8::Persistent<v8::Object>::New(wrapper);

    if (!hasDependentLifetime)
        wrapperHandle.MarkIndependent();
    V8DOMWrapper::setJSWrapperForDOMObject(impl, wrapperHandle, isolate);
    return wrapper;
}

void V8TestActiveDOMObject::derefObject(void* object)
{
    static_cast<TestActiveDOMObject*>(object)->deref();
}

} // namespace WebCore
