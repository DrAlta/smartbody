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

#ifndef ACTIVEMQ_CMSUTIL_MESSAGECREATOR_H
#define ACTIVEMQ_CMSUTIL_MESSAGECREATOR_H

#include <cms/Session.h>
#include <cms/Message.h>
#include <activemq/util/Config.h>

namespace activemq {
namespace cmsutil {

    /**
     * Creates the user-defined message to be sent by the
     * <code>CmsTemplate</code>.
     */
    class AMQCPP_API MessageCreator {
    public:

        virtual ~MessageCreator() throw () {}

        /**
         * Creates a message from the given session.
         *
         * @param session
         *          the CMS <code>Session</code>
         *
         * @throws cms::CMSException if thrown by CMS API methods
         */
        virtual cms::Message* createMessage( cms::Session* session ) = 0;

    };

}}

#endif /*ACTIVEMQ_CMSUTIL_MESSAGECREATOR_H*/
