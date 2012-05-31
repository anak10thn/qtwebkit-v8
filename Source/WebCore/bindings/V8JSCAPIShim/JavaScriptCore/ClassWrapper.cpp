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
#include "ClassWrapper.h"

using namespace v8;

namespace V8JSCAPIShim {

ClassWrapper::~ClassWrapper()
{
    for (unsigned i = 0; i < m_staticFunctions.size(); ++i) {
        Persistent<Value>& func = m_staticFunctions[i];
        ASSERT(!func.IsEmpty());
        func.Dispose();
        func.Clear();
    }
    for (unsigned i = 0; i < m_staticValueCallbacksList.size(); ++i) {
        Persistent<Value>& callbacks = m_staticValueCallbacksList[i];
        ASSERT(!callbacks.IsEmpty());
        StaticValueCallbacks* rawPtr = bitwise_cast<StaticValueCallbacks*>(External::Unwrap(callbacks));
        delete rawPtr;
        callbacks.Dispose();
        callbacks.Clear();
    }
    for (unsigned i = 0; i < m_propertyCallbacksList.size(); ++i) {
        Persistent<Value>& callbacks = m_propertyCallbacksList[i];
        ASSERT(!callbacks.IsEmpty());
        NamedPropertyCallbacks* rawPtr = bitwise_cast<NamedPropertyCallbacks*>(External::Unwrap(callbacks));
        delete rawPtr;
        callbacks.Dispose();
        callbacks.Clear();
    }
    ASSERT(!m_functionT.IsEmpty());
    m_functionT.Dispose();
    m_functionT.Clear();
}

} // namespace V8JSCAPIShim
