/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DECAF_UTIL_CONCURRENT_SYNCHRONIZABLE_H_
#define _DECAF_UTIL_CONCURRENT_SYNCHRONIZABLE_H_

#include <decaf/lang/exceptions/RuntimeException.h>
#include <decaf/lang/exceptions/InterruptedException.h>
#include <decaf/lang/exceptions/IllegalArgumentException.h>
#include <decaf/lang/exceptions/IllegalMonitorStateException.h>
#include <decaf/util/Config.h>

namespace decaf{
namespace util{
namespace concurrent{

    /**
     * The interface for all synchronizable objects (that is, objects
     * that can be locked and unlocked).
     *
     * @since 1.0
     */
    class DECAF_API Synchronizable {
    public:

        virtual ~Synchronizable() {}

        /**
         * Locks the object.
         *
         * @throws RuntimeException if an error occurs while locking the object.
         */
        virtual void lock() = 0;

        /**
         * Attempts to Lock the object, if the lock is already held by another
         * thread than this method returns false.
         *
         * @return true if the lock was acquired, false if it is already held by
         *         another thread.
         *
         * @throws RuntimeException if an error occurs while locking the object.
         */
        virtual bool tryLock() = 0;

        /**
         * Unlocks the object.
         *
         * @throws RuntimeException if an error occurs while unlocking the object.
         */
        virtual void unlock() = 0;

        /**
         * Waits on a signal from this object, which is generated
         * by a call to Notify.  Must have this object locked before
         * calling.
         *
         * @throws RuntimeException if an error occurs while waiting on the object.
         * @throws InterruptedException if the wait is interrupted before it completes.
         * @throws IllegalMonitorStateException - if the current thread is not the owner of the
         *         the Synchronizable Object.
         */
        virtual void wait() = 0;

        /**
         * Waits on a signal from this object, which is generated
         * by a call to Notify.  Must have this object locked before
         * calling.  This wait will timeout after the specified time
         * interval.
         *
         * @param millisecs
         *      the time in milliseconds to wait, or WAIT_INIFINITE
         *
         * @throws RuntimeException if an error occurs while waiting on the object.
         * @throws InterruptedException if the wait is interrupted before it completes.
         * @throws IllegalMonitorStateException - if the current thread is not the owner of the
         *         the Synchronizable Object.
         */
        virtual void wait( long long millisecs ) = 0;

        /**
         * Waits on a signal from this object, which is generated by a call to Notify.
         * Must have this object locked before calling.  This wait will timeout after the
         * specified time interval.  This method is similar to the one argument wait function
         * except that it add a finer grained control over the amount of time that it waits
         * by adding in the additional nanosecond argument.
         *
         * NOTE: The ability to wait accurately at a nanosecond scale depends on the platform
         * and OS that the Decaf API is running on, some systems do not provide an accurate
         * enough clock to provide this level of granularity.
         *
         * @param millisecs
         *      the time in milliseconds to wait, or WAIT_INIFINITE
         * @param nanos
         *      additional time in nanoseconds with a range of 0-999999
         *
         * @throws IllegalArgumentException if an error occurs or the nanos argument is not in
         *         the range of [0-999999]
         * @throws RuntimeException if an error occurs while waiting on the object.
         * @throws InterruptedException if the wait is interrupted before it completes.
         * @throws IllegalMonitorStateException - if the current thread is not the owner of the
         *         the Synchronizable Object.
         */
        virtual void wait( long long millisecs, int nanos ) = 0;

        /**
         * Signals a waiter on this object that it can now wake
         * up and continue.  Must have this object locked before
         * calling.
         *
         * @throws IllegalMonitorStateException - if the current thread is not the owner of the
         *         the Synchronizable Object.
         * @throws RuntimeException if an error occurs while notifying one of the waiting threads.
         */
        virtual void notify() = 0;

        /**
         * Signals the waiters on this object that it can now wake
         * up and continue.  Must have this object locked before
         * calling.
         *
         * @throws IllegalMonitorStateException - if the current thread is not the owner of the
         *         the Synchronizable Object.
         * @throws RuntimeException if an error occurs while notifying the waiting threads.
         */
        virtual void notifyAll() = 0;

    };

}}}

#endif /*_DECAF_UTIL_CONCURRENT_SYNCHRONIZABLE_H_*/
