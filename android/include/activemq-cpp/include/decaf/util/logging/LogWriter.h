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
#ifndef _DECAF_UTIL_LOGGING_LOGWRITER_H_
#define _DECAF_UTIL_LOGGING_LOGWRITER_H_

#include <decaf/util/concurrent/Mutex.h>

namespace decaf{
namespace util{
namespace logging{

    class DECAF_API LogWriter {
    public:

        LogWriter();
        virtual ~LogWriter();

        /**
         * Writes a message to the output destination
         * @param file
         * @param line
         * @param prefix
         * @param message
         */
        virtual void log( const std::string& file,
                          const int          line,
                          const std::string& prefix,
                          const std::string& message );

        /**
         * Writes a message to the output destination
         * @param message
         */
        virtual void log(const std::string& message);

    public:    // Static methods

        /**
         * Get the singleton instance
         */
        static LogWriter& getInstance();

        /**
         * Returns a Checked out instance of this Writer
         */
        static void returnInstance();

        /**
         * Forcefully Delete the Instance of this LogWriter
         * even if there are outstanding references.
         */
        static void destroy();

    private:

        /**
         * Gets the one and only mutex for this instance.
         */
        concurrent::Mutex& getMutex();

    };

}}}

#endif /*_DECAF_UTIL_LOGGING_LOGWRITER_H_*/
