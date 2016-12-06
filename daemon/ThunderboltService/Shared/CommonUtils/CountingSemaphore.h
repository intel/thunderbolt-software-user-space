/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2015 - 2016 Intel Corporation.
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

#include <condition_variable>

namespace Utils
{
/**
 * \brief CountingSemaphore purpose is to use as a better condition variable to some usages.
 * It allows accumulating the notify() actions and thus solves race condition when the waiting
 * side is starting to wait only after the notifier side already notified the condition variable.
 *
 * Please note: Currently this is not a real counting semaphore in the general meaning, as it
 * doesn't allow initial count to allow the firs few waiters to not wait. If we need it, we can
 * add this option, too.
 *
 * If the notification count gets to more than std::numeric_limits<unsigned int>::max(), an overflow
 * happens and you get strange behavior. But it's OK, you deserve it for having so many waiting
 * notifications...
 *
 * As with std::condition_variable, it isn't specified which of the waiting thread will get notified
 * when you call notify().
 *
 * The interface imitates std::condition_variable's interface, but there are some differences:
 * 1. You don't supply your own mutex, as you have to do with std::condition_variable::wait*()
 *    functions. This influence the wait*() function signature.
 * 2. You can use lock() to get a lock on the internal used mutex, to protect other things that
 *    are logically related to the semaphore. You are supposed to move this lock then to wait*()
 *    or notify*() functions. Otherwise, expect a dead lock.
 * 3. We actually use a predicate for wait*() functions even if one wasn't supplied by the user,
 *    to track the notification count. This means that for wait_for() / wait_until() we always
 *    return bool, never std::cv_status.
 * 4. For the variation of wait*() functions that takes predicate, the actual predicate for the
 *    waiting is the supplied predicate && our internal one. It means that even if the condition
 *    supplied by you is fulfilled, the wait continues until the notification number isn't 0
 *    (which is a Good Thing(TM)!).
 * 5. You get the lock back from all the functions. As long as you keep holding it (or the lock you
 *    acquired from lock()), no waiting thread can continue with its wake-up process and no notify
 *    can happen. Release it as soon as you can or don't hold it in first place (just ignore the
 *    return value).
 *    Another effect of holding the lock for too long is that callers that specified
 *    timeout (wait_for() / wait_until() callers) can wait much more for taking the lock (and only
 *    then the timeout is in effect when waiting on the condition variable).
 *
 * \todo
 * - Find a better way than using lock(). Currently a user can enter dead lock too easily, which is
 *   bad.
 * - Better interface for letting the user to decide if the return of the lock is needed. Making it
 *   explicit can solve incidentally holding the returned lock. It's especially bad with wait_for() /
 *   wait_until() functions, where the user usually has to hold the return value to check the bool.
 * - Consider changing the mutex to timed mutex, and make sure that caller of wait_for() / wait_until()
 *   will not get blocked for more than the timeout specified.
 */
class CountingSemaphore
{
   using MutexType = std::mutex;

public:
   using LockType = std::unique_lock<MutexType>;

   /**
    * \brief Similar to std::condition_variable::notify_one(). For lock parameter, see lock().
    *
    * Returns the internal lock as explained in #5 in class documentation.
    */
   LockType notify(LockType lock = LockType());

   /**
    * \brief Similar to std::condition_variable::wait(). For lock parameter, see lock().
    *
    * Returns the internal lock as explained in #5 in class documentation.
    */
   LockType wait(LockType lock = LockType()) { return wait(m_pred, std::move(lock)); }

   /**
    * \brief Similar to std::condition_variable::wait(). For lock parameter, see lock().
    *
    * Returns the internal lock as explained in #5 in class documentation.
    */
   template <class Predicate>
   LockType wait(Predicate pred, LockType lock = LockType());

   /**
    * \brief Similar to std::condition_variable::wait_for(). For lock parameter, see lock().
    *
    * Returns bool as in std::condition_variable::wait_for() together with the internal lock as
    * explained in #5 in class documentation.
    */
   template <class Rep, class Period>
   std::pair<bool, LockType> wait_for(const std::chrono::duration<Rep, Period>& rel_time, LockType lock = LockType())
   {
      return wait_for(rel_time, m_pred, std::move(lock));
   }

   /**
    * \brief Similar to std::condition_variable::wait_for(). For lock parameter, see lock().
    *
    * Returns bool as in std::condition_variable::wait_for() together with the internal lock as
    * explained in #5 in class documentation.
    */
   template <class Rep, class Period, class Predicate>
   std::pair<bool, LockType>
   wait_for(const std::chrono::duration<Rep, Period>& rel_time, Predicate pred, LockType lock = LockType());

   /**
    * \brief Similar to std::condition_variable::wait_for(). For lock parameter, see lock().
    *
    * Returns bool as in std::condition_variable::wait_for() together with the internal lock as
    * explained in #5 in class documentation.
    */
   template <class Clock, class Duration>
   std::pair<bool, LockType> wait_until(const std::chrono::time_point<Clock, Duration>& timeout_time,
                                        LockType lock = LockType())
   {
      return wait_until(timeout_time, m_pred, std::move(lock));
   }

   /**
    * \brief Similar to std::condition_variable::wait_for(). For lock parameter, see lock().
    *
    * Returns bool as in std::condition_variable::wait_for() together with the internal lock as
    * explained in #5 in class documentation.
    */
   template <class Clock, class Duration, class Predicate>
   std::pair<bool, LockType>
   wait_until(const std::chrono::time_point<Clock, Duration>& timeout_time, Predicate pred, LockType lock = LockType())
   {
      return wait_for(timeout_time - Clock::now(), pred, std::move(lock));
   }

   /**
    * \brief Acquire the internal used lock to protect other things that has to be synchronized with the
    * semaphore.
    *
    * You are supposed to move this lock then as an argument to wait*() or notify*()
    * functions. Otherwise, expect a dead lock.
    * See also #2 & #5 items in class documentation.
    */
   LockType lock() { return LockType(m_cvMutex); }

   /**
    * \brief Reset the notification count
    *
    * Useful when you want to ignore previous notifications for some reason.
    */
   void reset();

private:
   MutexType m_cvMutex; // the main mutex here, the one that protects the cv and the notification
   std::condition_variable m_cv;
   unsigned m_notificationCount = 0;

   /**
    * \brief Used internally as the predicate for wait*() functions
    */
   struct InternalPred
   {
      InternalPred(unsigned& notificationCount) : m_notificationCount(notificationCount) {}

      // No lock here. The public interface that uses it has to guard the access.
      bool operator()() { return m_notificationCount != 0; }

   private:
      unsigned& m_notificationCount;
   };
   InternalPred m_pred{m_notificationCount};

   /**
    * \brief Used internally to make sure we have the lock, no matter if we got it from the user
    * (who used lock()) or we need to lock it ourselves
    */
   LockType internalLock(LockType lock);
};

// ------------------ template member function implementation --------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <class Predicate>
auto CountingSemaphore::wait(Predicate pred, LockType lock) -> LockType
{
   auto l = internalLock(std::move(lock));
   m_cv.wait(l, [&pred, this] { return m_pred() && pred(); });
   --m_notificationCount;
   return l;
}

template <class Rep, class Period, class Predicate>
auto CountingSemaphore::wait_for(const std::chrono::duration<Rep, Period>& rel_time, Predicate pred, LockType lock)
   -> std::pair<bool, LockType>
{
   auto l   = internalLock(std::move(lock));
   auto res = m_cv.wait_for(l, rel_time, [&pred, this] { return m_pred() && pred(); });
   if (res)
   {
      --m_notificationCount;
   }
   return std::make_pair(res, std::move(l));
}
#endif // DOXYGEN_SHOULD_SKIP_THIS

} // namespace Utils
