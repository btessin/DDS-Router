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
 * @file EchoParticipant.hpp
 */

#ifndef __SRC_DDSROUTERCORE_PARTICIPANT_IMPLEMENTATIONS_AUXILIAR_ECHOPARTICIPANT_HPP_
#define __SRC_DDSROUTERCORE_PARTICIPANT_IMPLEMENTATIONS_AUXILIAR_ECHOPARTICIPANT_HPP_

#include <ddsrouter_core/participants/participant/configuration/EchoParticipantConfiguration.hpp>

#include <ddsrouter_core/participants/participant/auxiliar/BlankParticipant.hpp>

namespace eprosima {
namespace ddsrouter {
namespace participants {

/**
 * Concrete Participant that prints in stdout each message that arrives.
 */
class EchoParticipant : public BlankParticipant
{
public:

    //! Using parent class constructors
    EchoParticipant(
            std::shared_ptr<EchoParticipantConfiguration> participant_configuration,
            std::shared_ptr<core::DiscoveryDatabase> discovery_database);

    //! Print discovery information from endpoint discovered
    void echo_discovery(
            core::types::Endpoint endpoint_discovered) const noexcept;

    //! Override create_writer() IParticipant method
    std::shared_ptr<core::IWriter> create_writer(
            core::types::DdsTopic topic) override;

protected:

    // Deleters do not need to be implemented

    //! Reference to alias access of this object configuration without casting every time
    std::shared_ptr<EchoParticipantConfiguration> configuration_;

    //! DDS Router shared Discovery Database
    std::shared_ptr<core::DiscoveryDatabase> discovery_database_;
};

} /* namespace participants */
} /* namespace ddsrouter */
} /* namespace eprosima */

#endif /* __SRC_DDSROUTERCORE_PARTICIPANT_IMPLEMENTATIONS_AUXILIAR_ECHOPARTICIPANT_HPP_ */