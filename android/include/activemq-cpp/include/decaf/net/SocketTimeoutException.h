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

#ifndef _DECAF_NET_SOCKETTIMEOUTEXCEPTION_H_
#define _DECAF_NET_SOCKETTIMEOUTEXCEPTION_H_

#include <decaf/util/Config.h>
#include <decaf/io/InterruptedIOException.h>

namespace decaf{
namespace net{

    class DECAF_API SocketTimeoutException : public io::InterruptedIOException {
    public:

        /**
         * Default Constructor
         */
        SocketTimeoutException() throw() {}

        /**
         * Conversion Constructor from some other Exception
         * @param ex An exception that should become this type of Exception
         */
        SocketTimeoutException( const Exception& ex ) throw()
        : io::InterruptedIOException()
        {
            *(Exception*)this = ex;
        }

        /**
         * Copy Constructor
         * @param ex An exception that should become this type of Exception
         */
        SocketTimeoutException( const SocketTimeoutException& ex ) throw()
        : io::InterruptedIOException()
        {
            *(Exception*)this = ex;
        }

        /**
         * Constructor - Initializes the file name and line number where
         * this message occurred.  Sets the message to report, using an
         * optional list of arguments to parse into the message
         * @param file The file name where exception occurs
         * @param lineNumber The line number where the exception occurred.
         * @param cause The exception that was the cause for this one to be thrown.
         * @param msg The message to report
         * @param ... list of primitives that are formatted into the message
         */
        SocketTimeoutException( const char* file, const int lineNumber,
                                const std::exception* cause,
                                const char* msg, ... )
        throw() : io::InterruptedIOException( cause )
        {
            va_list vargs;
            va_start( vargs, msg );
            buildMessage( msg, vargs );

            // Set the first mark for this exception.
            setMark( file, lineNumber );
        }

        /**
         * Constructor
         * @param cause Pointer to the exception that caused this one to
         * be thrown, the object is cloned caller retains ownership.
         */
        SocketTimeoutException( const std::exception* cause )
            throw() : io::InterruptedIOException( cause ) {}

        /**
         * Constructor - Initializes the file name and line number where
         * this message occurred.  Sets the message to report, using an
         * optional list of arguments to parse into the message
         * @param file The file name where exception occurs
         * @param lineNumber The line number where the exception occurred.
         * @param msg The message to report
         * @param ... list of primitives that are formatted into the message
         */
        SocketTimeoutException( const char* file, const int lineNumber,
                                const char* msg, ... ) throw ()
        : io::InterruptedIOException()
        {
            va_list vargs ;
            va_start( vargs, msg );
            buildMessage( msg, vargs );

            // Set the first mark for this exception.
            setMark( file, lineNumber );
        }

        /**
         * Clones this exception.  This is useful for cases where you need
         * to preserve the type of the original exception as well as the message.
         * All subclasses should override.
         *
         * @return a new Exception instance that is a copy of this Exception object.
         */
        virtual SocketTimeoutException* clone() const {
            return new SocketTimeoutException( *this );
        }

        virtual ~SocketTimeoutException() throw() {}

    };

}}

#endif /*_DECAF_NET_SOCKETTIMEOUTEXCEPTION_H_*/
