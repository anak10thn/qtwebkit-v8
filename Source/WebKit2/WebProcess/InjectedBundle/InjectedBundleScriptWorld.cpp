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

#include "config.h"
#include "InjectedBundleScriptWorld.h"

#include <WebCore/DOMWrapperWorld.h>
#include <WebCore/ScriptController.h>
#include <wtf/HashMap.h>

#if USE(V8)
#include <WebCore/V8Binding.h>
#include <WebCore/V8DOMMap.h>
#include <WebCore/V8IsolatedContext.h>
#include <WebCore/V8Proxy.h>
#endif

using namespace WebCore;

namespace WebKit {

typedef HashMap<DOMWrapperWorld*, InjectedBundleScriptWorld*> WorldMap;

static WorldMap& allWorlds()
{
    DEFINE_STATIC_LOCAL(WorldMap, map, ());
    return map;
}

#if USE(V8)
static int s_isolatedWorldID = 0;
#endif

PassRefPtr<InjectedBundleScriptWorld> InjectedBundleScriptWorld::create()
{
#if USE(JSC)
    return adoptRef(new InjectedBundleScriptWorld(ScriptController::createWorld()));
#elif USE(V8)
    V8BindingPerIsolateData::ensureInitialized(v8::Isolate::GetCurrent());
    return adoptRef(new InjectedBundleScriptWorld(IsolatedWorld::create(s_isolatedWorldID++)));
#endif
}

PassRefPtr<InjectedBundleScriptWorld> InjectedBundleScriptWorld::getOrCreate(DOMWrapperWorld* world)
{
    if (world == mainThreadNormalWorld())
        return normalWorld();

    if (InjectedBundleScriptWorld* existingWorld = allWorlds().get(world))
        return existingWorld;

    return adoptRef(new InjectedBundleScriptWorld(world));
}

InjectedBundleScriptWorld* InjectedBundleScriptWorld::normalWorld()
{
    static InjectedBundleScriptWorld* world = adoptRef(new InjectedBundleScriptWorld(mainThreadNormalWorld())).leakRef();
    return world;
}

InjectedBundleScriptWorld::InjectedBundleScriptWorld(PassRefPtr<DOMWrapperWorld> world)
    : m_world(world)
{
    ASSERT(!allWorlds().contains(m_world.get()));
    allWorlds().add(m_world.get(), this);
}

InjectedBundleScriptWorld::~InjectedBundleScriptWorld()
{
    ASSERT(allWorlds().contains(m_world.get()));
    allWorlds().remove(m_world.get());
}

DOMWrapperWorld* InjectedBundleScriptWorld::coreWorld() const
{
    return m_world.get();
}
    
void InjectedBundleScriptWorld::clearWrappers()
{
#if USE(JSC)
    m_world->clearWrappers();
#elif USE(V8)
    ASSERT(allWorlds().contains(m_world.get()));
    allWorlds().remove(m_world.get());
    m_world = IsolatedWorld::create(s_isolatedWorldID++);
    allWorlds().add(m_world.get(), this);
#endif
}

} // namespace WebKit
