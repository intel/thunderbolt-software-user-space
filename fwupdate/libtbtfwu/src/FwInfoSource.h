/********************************************************************************
 * Thunderbolt(TM) FW update library
 * This library is distributed under the following BSD-style license:
 *
 * Copyright(c) 2016 Intel Corporation.
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

#include <memory>
#include <vector>
namespace tbt
{

namespace fwu
{

// Be careful: Don't reorder the items here; some functions count on the
// correct order here for comparing generations!
enum HwGeneration
{
   HW_GEN_UNKNOWN,

   /// <summary>Redwood Ridge</summary>
   DSL4510_4410,

   /// <summary>Falcon Ridge</summary>
   DSL5520_5320,

   /// <summary>Thunderbolt LP - Win Ridge</summary>
   DSL5110,

   /// <summary>Alpine Ridge</summary>
   DSL6540_6340,

   /// <summary>Alpine Ridge LP</summary>
   JHL6240,

   /// <summary>Alpine Ridge C</summary>
   JHL6540_6340,
};

enum HwType
{
   _2Ports,
   _1Port,
};

class HwInfo
{
public:
   HwInfo(HwGeneration gen)
#ifdef NDEBUG
      noexcept
#endif
      : m_Generation(gen),
        m_pHwType(nullptr)
   {
#ifndef NDEBUG
      check();
#endif
   }
   HwInfo(HwGeneration gen, HwType type) : m_Generation(gen), m_pHwType(new HwType(type))
   {
#ifndef NDEBUG
      check();
#endif
   }
   void swap(HwInfo& other) noexcept
   {
      std::swap(m_Generation, other.m_Generation);
      m_pHwType.swap(other.m_pHwType);
   }
   HwGeneration GetGeneration() const noexcept { return m_Generation; }
   void SetGeneration(HwGeneration gen) { m_Generation = gen; }
   bool HwTypeEquals(HwType type) const noexcept
   {
      if (!m_pHwType)
      {
         return false;
      }
      return *m_pHwType == type;
   }
   const HwType* GetHwType() const noexcept { return m_pHwType.get(); }
   void setHwType(HwType type) { m_pHwType.reset(new HwType(type)); }
   void clearHwType() noexcept { m_pHwType.reset(nullptr); }
private:
   HwGeneration m_Generation;
   std::unique_ptr<HwType> m_pHwType;
#ifndef NDEBUG
   static void check();
#endif
};

/// <summary>
/// Strategy class tree for abstracting the actual info extracting (reading), which can come
/// from a controller or from an image file, for FwInfo class tree
///
/// See ControllerFwInfoSource and FileFwInfoSource classes for the concrete classes
/// </summary>
class FwInfoSource
{
public:
   virtual std::vector<uint8_t> Read(uint32_t offset, uint32_t length) = 0;
   virtual uint32_t DigitalSectionOffset() = 0;
   const HwInfo& Info() const noexcept { return m_HwInfo; }
   static std::unique_ptr<HwInfo> HwConfiguration(uint16_t controllerId);
   FwInfoSource() noexcept;

protected:
   HwInfo& Info() noexcept { return m_HwInfo; }

   /// <summary>
   /// Reads the HW generation and type (port count) from FW (from controller or FW image)
   /// </summary>
   /// <returns>
   /// HWInfo object with generation and type (port count) info in regular case
   /// </returns>
   /// <exception>Throws in case of unknown device ID</exception>
   /// <remarks>
   /// We call it from each derived class c-tor, not from base class c-tor, because the use of
   /// abstract functions Read() and DigitalSectionOffset(), which may need the derived class
   /// initialized
   /// </remarks>
   std::unique_ptr<HwInfo> GetHwConfiguration();

private:
   HwInfo m_HwInfo;
};
}
}
