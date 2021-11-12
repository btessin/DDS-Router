// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file RealTopic.hpp
 */

#ifndef _DATABROKER_TYPES_TOPIC_REALTOPIC_HPP_
#define _DATABROKER_TYPES_TOPIC_REALTOPIC_HPP_

#include <databroker/types/topic/DatabrokerTopic.hpp>

namespace eprosima {
namespace databroker {

/**
 * Class that represents a real topic working in a DDS network
 */
struct RealTopic : public DatabrokerTopic
{
    //! Inherit parent constructors
    using DatabrokerTopic::DatabrokerTopic;

    /**
     * Whether a topic name and topic type name could be used as a valid real DDS topic
     *
     * @param topic_name: topic name
     * @param type_name: topic type name
     *
     * @return: true if a topic can be formed by these values
     */
    static bool is_real_topic(
            const std::string& topic_name,
            const std::string& type_name) noexcept;
};

} /* namespace databroker */
} /* namespace eprosima */

#endif /* _DATABROKER_TYPES_TOPIC_REALTOPIC_HPP_ */