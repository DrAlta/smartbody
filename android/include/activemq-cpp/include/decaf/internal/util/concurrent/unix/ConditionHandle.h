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


#ifndef _DECAF_UTIL_CONCURRENT_CONDITIONHANDLE_H_
#define _DECAF_UTIL_CONCURRENT_CONDITIONHANDLE_H_

#include <decaf/util/Config.h>

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif

namespace decaf {
namespace util {
namespace concurrent {

    class MutexHandle;

    class DECAF_API ConditionHandle {
    private:

        ConditionHandle( const ConditionHandle& );
        ConditionHandle& operator= ( const ConditionHandle& );

    public:

        ConditionHandle() : condition(), mutex(NULL) {
        }

        ~ConditionHandle() {
        }

        // The actual condition object
        pthread_cond_t condition;

        // The mutex object associated with this condition.
        MutexHandle* mutex;

    };

}}}

#endif /* _DECAF_UTIL_CONCURRENT_CONDITIONHANDLE_H_ */
