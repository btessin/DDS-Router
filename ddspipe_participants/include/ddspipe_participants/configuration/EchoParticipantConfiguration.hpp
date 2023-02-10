// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
// limitations under the License\.

#pragma once

#include <ddspipe_participants/configuration/ParticipantConfiguration.hpp>

namespace eprosima {
namespace ddspipe {
namespace participants {

struct EchoParticipantConfiguration : public ParticipantConfiguration
{

    //! Use default parent constructors
    using ParticipantConfiguration::ParticipantConfiguration;

    /////////////////////////
    // CONSTRUCTORS
    /////////////////////////
    DDSPIPE_PARTICIPANTS_DllAPI EchoParticipantConfiguration() = default;

    /////////////////////////
    // VARIABLES
    /////////////////////////

    //! Whether this Participant should echo the data received
    bool echo_data = false;
    //! Whether this Participant should echo the discovery information
    bool echo_discovery = true;
    //! Whether this Participant should echo verbose information
    bool verbose = false;
};

} /* namespace participants */
} /* namespace ddspipe */
} /* namespace eprosima */