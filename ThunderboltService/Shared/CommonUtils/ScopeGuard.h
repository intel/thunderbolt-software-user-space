/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2009 - 2016 Intel Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Intel Corporation nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************************/

#pragma once

#include <utility>

namespace Utils
{

/**
 * \brief Ad-hoc RAII utility class
 *
 * This class can be used for having an ad-hoc RAII behavior without the need
 * to write a dedicated wrapper class for each and every case we need RAII.
 *
 * This class is Movable and NonCopyable.
 *
 * Usually you want to create it using makeScopeGuard() helper function.
 *
 * disable() function is const (and thus m_enable member must be mutable) because
 * ScopeGuard is usually held as const.
 *
 * If you need to explicitly declare the variable (probably because you can't
 * initializing it yet), you probably want to use std::function as template
 * argument.
 *
 * Moving to VS2015, we'll be able to have versions that fire only on return or
 * only on exception (using std::uncaught_exceptions()).
 */
template <typename Callable>
class ScopeGuard
{
   template <typename Callable2>
   friend class ScopeGuard;

public:
   ScopeGuard();
   explicit ScopeGuard(Callable func);

   ScopeGuard(const ScopeGuard&) = delete;
   ScopeGuard& operator=(const ScopeGuard&) = delete;

   ScopeGuard(ScopeGuard&& other);
   ScopeGuard& operator=(ScopeGuard&& other);

   template <typename Callable2>
   ScopeGuard(ScopeGuard<Callable2>&& other);
   template <typename Callable2>
   ScopeGuard& operator=(ScopeGuard<Callable2>&& other);

   ~ScopeGuard();

   void disable() const { m_enabled = false; }
   bool isEnabled() const { return m_enabled; }

private:
   template <typename Callable2>
   ScopeGuard& doMove(ScopeGuard<Callable2>&& other);
   void cleanup();
   Callable m_func;
   mutable bool m_enabled = false;
};

template <typename Callable>
ScopeGuard<Callable>::ScopeGuard() = default;

template <typename Callable>
ScopeGuard<Callable>::ScopeGuard(Callable func) : m_func(func), m_enabled(true)
{
}

template <typename Callable>
ScopeGuard<Callable>::ScopeGuard(ScopeGuard&& other) : m_func(std::move(other.m_func)), m_enabled(other.m_enabled)
{
   other.disable();
}

template <typename Callable>
ScopeGuard<Callable>& ScopeGuard<Callable>::operator=(ScopeGuard&& other)
{
   return doMove(std::move(other));
}

template <typename Callable>
template <typename Callable2>
ScopeGuard<Callable>::ScopeGuard(ScopeGuard<Callable2>&& other)
   : m_func(std::move(other.m_func)), m_enabled(other.m_enabled)
{
   other.disable();
}

template <typename Callable>
template <typename Callable2>
ScopeGuard<Callable>& ScopeGuard<Callable>::operator=(ScopeGuard<Callable2>&& other)
{
   return doMove(std::move(other));
}

template <typename Callable>
ScopeGuard<Callable>::~ScopeGuard()
{
   cleanup();
}

template <typename Callable>
template <typename Callable2>
ScopeGuard<Callable>& ScopeGuard<Callable>::doMove(ScopeGuard<Callable2>&& other)
{
   cleanup();
   m_func    = std::move(other.m_func);
   m_enabled = other.m_enabled;
   other.disable();
   return *this;
}

template <typename Callable>
void ScopeGuard<Callable>::cleanup()
{
   if (isEnabled())
   {
      m_func();
   }
}

template <typename Callable>
ScopeGuard<Callable> makeScopeGuard(Callable func)
{
   return ScopeGuard<Callable>(func);
}

} // namespace Utils
