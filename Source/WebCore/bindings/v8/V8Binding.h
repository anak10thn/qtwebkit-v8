/*
* Copyright (C) 2009 Google Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*
*     * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
* copyright notice, this list of conditions and the following disclaimer
* in the documentation and/or other materials provided with the
* distribution.
*     * Neither the name of Google Inc. nor the names of its
* contributors may be used to endorse or promote products derived from
* this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef V8Binding_h
#define V8Binding_h

#include "BindingSecurity.h"
#include "DOMDataStore.h"
#include "PlatformString.h"
#include "V8DOMWrapper.h"
#include "V8GCController.h"
#include "V8HiddenPropertyName.h"
#include <wtf/MathExtras.h>
#include <wtf/Noncopyable.h>
#include <wtf/text/AtomicString.h>

#include <v8.h>

namespace WebCore {

    class DOMStringList;
    class DOMWrapperVisitor;
    class EventListener;
    class EventTarget;

    // FIXME: Remove V8Binding.
    class V8Binding {
    };
    typedef BindingSecurity<V8Binding> V8BindingSecurity;

    class StringCache {
    public:
        StringCache() { }

        v8::Local<v8::String> v8ExternalString(StringImpl* stringImpl) 
        {
            if (m_lastStringImpl.get() == stringImpl) {
                ASSERT(!m_lastV8String.IsNearDeath());
                ASSERT(!m_lastV8String.IsEmpty());
                return v8::Local<v8::String>::New(m_lastV8String);
            }

            return v8ExternalStringSlow(stringImpl);
        }

        void clearOnGC() 
        {
            m_lastStringImpl = 0;
            m_lastV8String.Clear();
        }

        void remove(StringImpl*);

    private:
        v8::Local<v8::String> v8ExternalStringSlow(StringImpl*);

        HashMap<StringImpl*, v8::String*> m_stringCache;
        v8::Persistent<v8::String> m_lastV8String;
        // Note: RefPtr is a must as we cache by StringImpl* equality, not identity
        // hence lastStringImpl might be not a key of the cache (in sense of identity)
        // and hence it's not refed on addition.
        RefPtr<StringImpl> m_lastStringImpl;
    };

    class ScriptGCEventListener;

    class GCEventData {
    public:
        typedef Vector<ScriptGCEventListener*> GCEventListeners;

        GCEventData() : startTime(0.0), usedHeapSize(0) { }
        void clear()
        {
            startTime = 0.0;
            usedHeapSize = 0;
        }
        GCEventListeners& listeners() { return m_listeners; }

        double startTime;
        size_t usedHeapSize;

    private:
        GCEventListeners m_listeners;
    };

    class ConstructorMode;

#ifndef NDEBUG
    typedef HashMap<v8::Value*, GlobalHandleInfo*> GlobalHandleMap;
#endif

    class V8BindingPerIsolateData {
    public:
        static V8BindingPerIsolateData* create(v8::Isolate*);
        static void ensureInitialized(v8::Isolate*);
        static V8BindingPerIsolateData* get(v8::Isolate* isolate)
        {
            ASSERT(isolate->GetData());
            return static_cast<V8BindingPerIsolateData*>(isolate->GetData()); 
        }

        static V8BindingPerIsolateData* current(v8::Isolate* isolate = 0)
        {
            return isolate ? static_cast<V8BindingPerIsolateData*>(isolate->GetData()) : get(v8::Isolate::GetCurrent());
        }
        static void dispose(v8::Isolate*);

        typedef HashMap<WrapperTypeInfo*, v8::Persistent<v8::FunctionTemplate> > TemplateMap;

        TemplateMap& rawTemplateMap() { return m_rawTemplates; }
        TemplateMap& templateMap() { return m_templates; }
        v8::Persistent<v8::String>& toStringName() { return m_toStringName; }
        v8::Persistent<v8::FunctionTemplate>& toStringTemplate() { return m_toStringTemplate; }

        v8::Persistent<v8::FunctionTemplate>& lazyEventListenerToStringTemplate()
        {
            return m_lazyEventListenerToStringTemplate;
        }

        StringCache* stringCache() { return &m_stringCache; }
#if ENABLE(INSPECTOR)
        void visitJSExternalStrings(DOMWrapperVisitor*);
#endif
        DOMDataList& allStores() { return m_domDataList; }

        V8HiddenPropertyName* hiddenPropertyName() { return &m_hiddenPropertyName; }
        v8::Persistent<v8::Context>& auxiliaryContext() { return m_auxiliaryContext; }

        void registerDOMDataStore(DOMDataStore* domDataStore) 
        {
            m_domDataList.append(domDataStore);
        }

        void unregisterDOMDataStore(DOMDataStore* domDataStore)
        {
            ASSERT(m_domDataList.find(domDataStore));
            m_domDataList.remove(m_domDataList.find(domDataStore));
        }


        DOMDataStore* domDataStore() { return m_domDataStore; }
        // DOMDataStore is owned outside V8BindingPerIsolateData.
        void setDOMDataStore(DOMDataStore* store) { m_domDataStore = store; }

        int recursionLevel() const { return m_recursionLevel; }
        int incrementRecursionLevel() { return ++m_recursionLevel; }
        int decrementRecursionLevel() { return --m_recursionLevel; }

#ifndef NDEBUG
        GlobalHandleMap& globalHandleMap() { return m_globalHandleMap; }

        int internalScriptRecursionLevel() const { return m_internalScriptRecursionLevel; }
        int incrementInternalScriptRecursionLevel() { return ++m_internalScriptRecursionLevel; }
        int decrementInternalScriptRecursionLevel() { return --m_internalScriptRecursionLevel; }
#endif

        GCEventData& gcEventData() { return m_gcEventData; }

    private:
        explicit V8BindingPerIsolateData(v8::Isolate*);
        ~V8BindingPerIsolateData();

        TemplateMap m_rawTemplates;
        TemplateMap m_templates;
        v8::Persistent<v8::String> m_toStringName;
        v8::Persistent<v8::FunctionTemplate> m_toStringTemplate;
        v8::Persistent<v8::FunctionTemplate> m_lazyEventListenerToStringTemplate;
        StringCache m_stringCache;

        DOMDataList m_domDataList;
        DOMDataStore* m_domDataStore;

        V8HiddenPropertyName m_hiddenPropertyName;
        v8::Persistent<v8::Context> m_auxiliaryContext;

        bool m_constructorMode;
        friend class ConstructorMode;

        int m_recursionLevel;

#ifndef NDEBUG
        GlobalHandleMap m_globalHandleMap;
        int m_internalScriptRecursionLevel;
#endif
        GCEventData m_gcEventData;
    };

    class ConstructorMode {
    public:
        enum Mode {
            WrapExistingObject,
            CreateNewObject
        };

        ConstructorMode()
        {
            V8BindingPerIsolateData* data = V8BindingPerIsolateData::current();
            m_previous = data->m_constructorMode;
            data->m_constructorMode = WrapExistingObject;
        }

        ~ConstructorMode()
        {
            V8BindingPerIsolateData* data = V8BindingPerIsolateData::current();
            data->m_constructorMode = m_previous;
        }

        static bool current() { return V8BindingPerIsolateData::current()->m_constructorMode; }

    private:
        bool m_previous;
    };


    enum ExternalMode {
        Externalize,
        DoNotExternalize
    };

    template <typename StringType>
    StringType v8StringToWebCoreString(v8::Handle<v8::String> v8String, ExternalMode external);

    // Since v8::Null(isolate) crashes if we pass a null isolate,
    // we need to instead use v8NullWithCheck(isolate).
    //
    // FIXME: Remove all null isolates from V8 bindings, and remove v8NullWithCheck(isolate).
    inline v8::Handle<v8::Value> v8NullWithCheck(v8::Isolate* isolate)
    {
        return isolate ? v8::Null(isolate) : v8::Null();
    }

    // Convert v8 types to a WTF::String. If the V8 string is not already
    // an external string then it is transformed into an external string at this
    // point to avoid repeated conversions.
    inline String v8StringToWebCoreString(v8::Handle<v8::String> v8String)
    {
        return v8StringToWebCoreString<String>(v8String, Externalize);
    }
    String v8NonStringValueToWebCoreString(v8::Handle<v8::Value>);
    String v8ValueToWebCoreString(v8::Handle<v8::Value> value);

    // Convert v8 types to a WTF::AtomicString.
    inline AtomicString v8StringToAtomicWebCoreString(v8::Handle<v8::String> v8String)
    {
        return v8StringToWebCoreString<AtomicString>(v8String, Externalize);
    }
    AtomicString v8NonStringValueToAtomicWebCoreString(v8::Handle<v8::Value>);
    AtomicString v8ValueToAtomicWebCoreString(v8::Handle<v8::Value> value);

    // Return a V8 external string that shares the underlying buffer with the given
    // WebCore string. The reference counting mechanism is used to keep the
    // underlying buffer alive while the string is still live in the V8 engine.
    inline v8::Local<v8::String> v8ExternalString(const String& string, v8::Isolate* isolate = 0)
    {
        StringImpl* stringImpl = string.impl();
        if (!stringImpl)
            return v8::String::Empty();

        V8BindingPerIsolateData* data = V8BindingPerIsolateData::current(isolate);
        return data->stringCache()->v8ExternalString(stringImpl);
    }

    // Convert a string to a V8 string.
    inline v8::Handle<v8::String> v8String(const String& string, v8::Isolate* isolate = 0)
    {
        return v8ExternalString(string, isolate);
    }

    template<typename T>
    v8::Handle<v8::Value> v8Array(const Vector<T>& iterator, v8::Isolate* isolate)
    {
        v8::Local<v8::Array> result = v8::Array::New(iterator.size());
        int index = 0;
        typename Vector<T>::const_iterator end = iterator.end();
        for (typename Vector<T>::const_iterator iter = iterator.begin(); iter != end; ++iter)
            result->Set(v8::Integer::New(index++), toV8(WTF::getPtr(*iter), isolate));
        return result;
    }

    template<>
    inline v8::Handle<v8::Value> v8Array(const Vector<String>& iterator, v8::Isolate* isolate)
    {
        v8::Local<v8::Array> array = v8::Array::New(iterator.size());
        Vector<String>::const_iterator end = iterator.end();
        int index = 0;
        for (Vector<String>::const_iterator iter = iterator.begin(); iter != end; ++iter)
            array->Set(v8::Integer::New(index++), v8String(*iter, isolate));
        return array;
    }

    template <class T>
    Vector<T> toNativeArray(v8::Handle<v8::Value> value)
    {
        if (!value->IsArray())
            return Vector<T>();

        Vector<T> result;
        v8::Local<v8::Value> v8Value(v8::Local<v8::Value>::New(value));
        v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(v8Value);
        size_t length = array->Length();

        for (size_t i = 0; i < length; ++i) {
            result.append(v8ValueToWebCoreString(array->Get(i)));
        }
        return result;
    }

    // Enables caching v8 wrappers created for WTF::StringImpl.  Currently this cache requires
    // all the calls (both to convert WTF::String to v8::String and to GC the handle)
    // to be performed on the main thread.
    void enableStringImplCache();

    // Convert a value to a 32-bit integer.  The conversion fails if the
    // value cannot be converted to an integer or converts to nan or to an infinity.
    int toInt32(v8::Handle<v8::Value> value, bool& ok);

    // Convert a value to a 32-bit integer assuming the conversion cannot fail.
    inline int toInt32(v8::Handle<v8::Value> value)
    {
        bool ok;
        return toInt32(value, ok);
    }

    // Convert a value to a 32-bit unsigned integer.  The conversion fails if the
    // value cannot be converted to an unsigned integer or converts to nan or to an infinity.
    uint32_t toUInt32(v8::Handle<v8::Value> value, bool& ok);

    // Convert a value to a 32-bit unsigned integer assuming the conversion cannot fail.
    inline uint32_t toUInt32(v8::Handle<v8::Value> value)
    {
        bool ok;
        return toUInt32(value, ok);
    }

    inline float toFloat(v8::Local<v8::Value> value)
    {
        return static_cast<float>(value->NumberValue());
    }

    inline long long toInt64(v8::Local<v8::Value> value)
    {
        return static_cast<long long>(value->IntegerValue());
    }

    // FIXME: Drop this in favor of the type specific v8ValueToWebCoreString when we rework the code generation.
    inline String toWebCoreString(v8::Handle<v8::Value> object)
    {
        return v8ValueToWebCoreString(object);
    }

    inline String toWebCoreString(const v8::Arguments& args, int index)
    {
        return v8ValueToWebCoreString(args[index]);
    }

    // The string returned by this function is still owned by the argument
    // and will be deallocated when the argument is deallocated.
    inline const uint16_t* fromWebCoreString(const String& str)
    {
        return reinterpret_cast<const uint16_t*>(str.characters());
    }

    inline bool isUndefinedOrNull(v8::Handle<v8::Value> value)
    {
        return value->IsNull() || value->IsUndefined();
    }

    // Returns true if the provided object is to be considered a 'host object', as used in the
    // HTML5 structured clone algorithm.
    inline bool isHostObject(v8::Handle<v8::Object> object)
    {
        // If the object has any internal fields, then we won't be able to serialize or deserialize
        // them; conveniently, this is also a quick way to detect DOM wrapper objects, because
        // the mechanism for these relies on data stored in these fields. We should
        // catch external array data as a special case.
        return object->InternalFieldCount() || object->HasIndexedPropertiesInExternalArrayData();
    }

    inline v8::Handle<v8::Boolean> v8Boolean(bool value)
    {
        return value ? v8::True() : v8::False();
    }

    inline String toWebCoreStringWithNullCheck(v8::Handle<v8::Value> value)
    {
        return value->IsNull() ? String() : v8ValueToWebCoreString(value);
    }

    inline AtomicString toAtomicWebCoreStringWithNullCheck(v8::Handle<v8::Value> value)
    {
        return value->IsNull() ? AtomicString() : v8ValueToAtomicWebCoreString(value);
    }

    inline String toWebCoreStringWithNullOrUndefinedCheck(v8::Handle<v8::Value> value)
    {
        return (value->IsNull() || value->IsUndefined()) ? String() : toWebCoreString(value);
    }

    inline v8::Handle<v8::String> v8UndetectableString(const String& str)
    {
        return v8::String::NewUndetectable(fromWebCoreString(str), str.length());
    }

    inline v8::Handle<v8::Value> v8StringOrNull(const String& str, v8::Isolate* isolate = 0)
    {
        return str.isNull() ? v8::Handle<v8::Value>(v8::Null()) : v8::Handle<v8::Value>(v8String(str, isolate));
    }

    inline v8::Handle<v8::Value> v8StringOrUndefined(const String& str, v8::Isolate* isolate = 0)
    {
        return str.isNull() ? v8::Handle<v8::Value>(v8::Undefined()) : v8::Handle<v8::Value>(v8String(str, isolate));
    }

    inline v8::Handle<v8::Value> v8StringOrFalse(const String& str, v8::Isolate* isolate = 0)
    {
        return str.isNull() ? v8::Handle<v8::Value>(v8::False()) : v8::Handle<v8::Value>(v8String(str, isolate));
    }

    template <class T> v8::Handle<v8::Value> v8NumberArray(const Vector<T>& values)
    {
        size_t size = values.size();
        v8::Local<v8::Array> result = v8::Array::New(size);
        for (size_t i = 0; i < size; ++i)
            result->Set(i, v8::Number::New(values[i]));
        return result;
    }

    inline double toWebCoreDate(v8::Handle<v8::Value> object)
    {
        return (object->IsDate() || object->IsNumber()) ? object->NumberValue() : std::numeric_limits<double>::quiet_NaN();
    }

    inline v8::Handle<v8::Value> v8DateOrNull(double value, v8::Isolate* isolate = 0)
    {
        return isfinite(value) ? v8::Date::New(value) : v8NullWithCheck(isolate);
    }

    v8::Persistent<v8::FunctionTemplate> createRawTemplate();

    struct BatchedAttribute;
    struct BatchedCallback;

    v8::Local<v8::Signature> configureTemplate(v8::Persistent<v8::FunctionTemplate>,
                                               const char* interfaceName,
                                               v8::Persistent<v8::FunctionTemplate> parentClass,
                                               int fieldCount,
                                               const BatchedAttribute*,
                                               size_t attributeCount,
                                               const BatchedCallback*,
                                               size_t callbackCount);

    v8::Handle<v8::Value> getElementStringAttr(const v8::AccessorInfo&,
                                               const QualifiedName&);
    void setElementStringAttr(const v8::AccessorInfo&,
                              const QualifiedName&,
                              v8::Local<v8::Value>);


    v8::Persistent<v8::String> getToStringName();
    v8::Persistent<v8::FunctionTemplate> getToStringTemplate();

    String int32ToWebCoreString(int value);

    template <class T> Vector<T> v8NumberArrayToVector(v8::Handle<v8::Value> value)
    {
        v8::Local<v8::Value> v8Value(v8::Local<v8::Value>::New(value));
        if (!v8Value->IsArray())
            return Vector<T>();

        Vector<T> result;
        v8::Local<v8::Array> v8Array = v8::Local<v8::Array>::Cast(v8Value);
        size_t length = v8Array->Length();
        for (size_t i = 0; i < length; ++i) {
            v8::Local<v8::Value> indexedValue = v8Array->Get(v8::Integer::New(i));
            result.append(static_cast<T>(indexedValue->NumberValue()));
        }
        return result;
    }

    PassRefPtr<DOMStringList> v8ValueToWebCoreDOMStringList(v8::Handle<v8::Value>);

    class V8ParameterBase {
    public:
        operator String() { return toString<String>(); }
        operator AtomicString() { return toString<AtomicString>(); }

    protected:
        V8ParameterBase(v8::Local<v8::Value> object) : m_v8Object(object), m_mode(Externalize), m_string() { }

        bool prepareBase()
        {
            if (m_v8Object.IsEmpty())
                return true;

            if (LIKELY(m_v8Object->IsString()))
                return true;

            if (LIKELY(m_v8Object->IsInt32())) {
                setString(int32ToWebCoreString(m_v8Object->Int32Value()));
                return true;
            }

            m_mode = DoNotExternalize;
            v8::TryCatch block;
            m_v8Object = m_v8Object->ToString();
            // Handle the case where an exception is thrown as part of invoking toString on the object.
            if (block.HasCaught()) {
                block.ReThrow();
                return false;
            }
            return true;
        }

        v8::Local<v8::Value> object() { return m_v8Object; }

        void setString(const String& string)
        {
            m_string = string;
            m_v8Object.Clear(); // To signal that String is ready.
        }

     private:
        v8::Local<v8::Value> m_v8Object;
        ExternalMode m_mode;
        String m_string;

        template <class StringType>
        StringType toString()
        {
            if (LIKELY(!m_v8Object.IsEmpty()))
                return v8StringToWebCoreString<StringType>(m_v8Object.As<v8::String>(), m_mode);

            return StringType(m_string);
        }
    };

    // V8Parameter is an adapter class that converts V8 values to Strings
    // or AtomicStrings as appropriate, using multiple typecast operators.
    enum V8ParameterMode {
        DefaultMode,
        WithNullCheck,
        WithUndefinedOrNullCheck
    };
    template <V8ParameterMode MODE = DefaultMode>
    class V8Parameter: public V8ParameterBase {
    public:
        V8Parameter(v8::Local<v8::Value> object) : V8ParameterBase(object) { }
        V8Parameter(v8::Local<v8::Value> object, bool) : V8ParameterBase(object) { prepare(); }

        bool prepare();
    };

    template<> inline bool V8Parameter<DefaultMode>::prepare()
    {
        return V8ParameterBase::prepareBase();
    }

    template<> inline bool V8Parameter<WithNullCheck>::prepare()
    {
        if (object().IsEmpty() || object()->IsNull()) {
            setString(String());
            return true;
        }

        return V8ParameterBase::prepareBase();
    }

    template<> inline bool V8Parameter<WithUndefinedOrNullCheck>::prepare()
    {
        if (object().IsEmpty() || object()->IsNull() || object()->IsUndefined()) {
            setString(String());
            return true;
        }

        return V8ParameterBase::prepareBase();
    }

    enum ParameterDefaultPolicy {
        DefaultIsUndefined,
        DefaultIsNullString
    };

} // namespace WebCore

#endif // V8Binding_h
