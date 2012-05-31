/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebSerializedScriptValue_h
#define WebSerializedScriptValue_h

#include "APIObject.h"
#include "DataReference.h"
#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/JavaScript.h>
#include <WebCore/SerializedScriptValue.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>

namespace WebKit {

class WebSerializedScriptValue : public APIObject {
public:
    static const Type APIType = TypeSerializedScriptValue;
    
    static PassRefPtr<WebSerializedScriptValue> create(WebCore::SerializedScriptValue* serializedValue)
    {
        return adoptRef(new WebSerializedScriptValue(serializedValue));
    }
    
    static PassRefPtr<WebSerializedScriptValue> create(JSContextRef context, JSValueRef value, JSValueRef* exception)
    {
#if USE(JSC)
        RefPtr<WebCore::SerializedScriptValue> serializedValue = WebCore::SerializedScriptValue::create(context, value, exception);
#elif USE(V8)
        RefPtr<WebCore::SerializedScriptValue> serializedValue = WebCore::SerializedScriptValue::create(toV8(value));
#endif
        if (!serializedValue)
            return 0;
        return adoptRef(new WebSerializedScriptValue(serializedValue.get()));
    }
    
    static PassRefPtr<WebSerializedScriptValue> adopt(Vector<uint8_t>& buffer)
    {
#if USE(JSC)
        return adoptRef(new WebSerializedScriptValue(WebCore::SerializedScriptValue::adopt(buffer)));
#elif USE(V8)
        String wireString(buffer.data(), buffer.size());
        return adoptRef(new WebSerializedScriptValue(WebCore::SerializedScriptValue::createFromWire(wireString)));
#endif
    }
    
    JSValueRef deserialize(JSContextRef context, JSValueRef* exception)
    {
#if USE(V8)
        return toRef(m_serializedScriptValue->deserialize());
#else
        return m_serializedScriptValue->deserialize(context, exception);
#endif
    }

    CoreIPC::DataReference dataReference() const
    {
#if USE(V8)
        if (!m_buffer.isEmpty())
            return m_buffer;

        String wireString = m_serializedScriptValue->toWireString();
        unsigned lengthInBytes = wireString.length();
        const uint8_t* characters;
        if (wireString.is8Bit())
            characters = static_cast<const uint8_t*>(wireString.characters8());
        else {
            lengthInBytes *= 2;
            characters = reinterpret_cast<const uint8_t*>(wireString.characters16());
        }
        m_buffer = Vector<uint8_t>(lengthInBytes);
        memcpy(m_buffer.data(), characters, lengthInBytes);
        return m_buffer;
#else
        return m_serializedScriptValue->data();
#endif
    }

    void* internalRepresentation() { return m_serializedScriptValue.get(); }

private:
    explicit WebSerializedScriptValue(PassRefPtr<WebCore::SerializedScriptValue> serializedScriptValue)
        : m_serializedScriptValue(serializedScriptValue)
    {
    }
    
    virtual Type type() const { return APIType; }
    
    RefPtr<WebCore::SerializedScriptValue> m_serializedScriptValue;
#if USE(V8)
    mutable Vector<uint8_t> m_buffer;
#endif
};
    
}

#endif // WebSerializedScriptValue_h
