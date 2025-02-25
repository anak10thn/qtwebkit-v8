/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#ifndef WebTextCheckingCompletionImpl_h
#define WebTextCheckingCompletionImpl_h

#include "WebTextCheckingCompletion.h"

namespace WebCore {
class SpellChecker;
}

namespace WebKit {

class EditorClientImpl;

class WebTextCheckingCompletionImpl : public WebTextCheckingCompletion {
public:
    WebTextCheckingCompletionImpl(int identifier, WebCore::SpellChecker* spellchecker, EditorClientImpl* editorClient)
        : m_identifier(identifier)
        , m_spellChecker(spellchecker)
        , m_editorClient(editorClient)
    {
    }

    virtual void didFinishCheckingText(const WebVector<WebTextCheckingResult>&);
    virtual void didCancelCheckingText();

    void invalidate();
    WebCore::SpellChecker* spellChecker() const { return m_spellChecker; }

private:
    virtual ~WebTextCheckingCompletionImpl() { }

    int m_identifier;
    WebCore::SpellChecker* m_spellChecker;
    EditorClientImpl* m_editorClient;
};

} // namespace WebKit

#endif
