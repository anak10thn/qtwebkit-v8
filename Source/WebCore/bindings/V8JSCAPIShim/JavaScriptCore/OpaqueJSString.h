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

#ifndef OpaqueJSString_h
#define OpaqueJSString_h

#include "JSStringRef.h"
#include <v8.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/text/StringImpl.h>

struct OpaqueJSString : public ThreadSafeRefCounted<OpaqueJSString> {

    static PassRefPtr<OpaqueJSString> create() // null
    {
        return adoptRef(new OpaqueJSString);
    }

    static PassRefPtr<OpaqueJSString> create(const UChar* characters, unsigned length)
    {
        return adoptRef(new OpaqueJSString(characters, length));
    }

    static PassRefPtr<OpaqueJSString> create(v8::Handle<v8::String> v8String)
    {
        return PassRefPtr<OpaqueJSString>(new OpaqueJSString(v8String));
    }

    UChar* characters() const { return this ? m_characters : 0; }
    unsigned length() const { return this ? m_length : 0; }

    v8::Handle<v8::String> toV8String() const
    {
        if (characters())
            return v8::String::New(m_characters, m_length);
        return v8::String::Empty();
    }

#ifndef NDEBUG
    NEVER_INLINE const char* toUtf8()
    {
        v8::HandleScope handleScope;
        v8::Handle<v8::String> v8Str = toV8String();
        return *v8::String::Utf8Value(v8Str);
    }
#endif

private:
    friend class WTF::ThreadSafeRefCounted<OpaqueJSString>;

    OpaqueJSString()
        : m_characters(0)
        , m_length(0)
    {
    }

    OpaqueJSString(const UChar* characters, unsigned length)
        : m_length(length)
    {
        m_characters = new UChar[length];
        memcpy(m_characters, characters, length * sizeof(UChar));
    }

    OpaqueJSString(v8::Handle<v8::String> v8String)
        : m_length(v8String->Length())
    {
        m_characters = new UChar[m_length];
        v8String->Write(static_cast<uint16_t*>(m_characters), 0, m_length);
    }

    ~OpaqueJSString()
    {
        delete[] m_characters;
    }

    UChar* m_characters;
    unsigned m_length;
};

#endif
