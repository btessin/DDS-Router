// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ParticipantFactory.cpp
 *
 */

#include <cpp_utils/exception/ConfigurationException.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/interface/IParticipant.hpp>
#include <ddspipe_participants/configuration/DiscoveryServerParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/EchoParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/InitialPeersParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/ParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/SimpleParticipantConfiguration.hpp>
#include <ddspipe_participants/participant/auxiliar/EchoParticipant.hpp>
#include <ddspipe_participants/participant/rtps/DiscoveryServerParticipant.hpp>
#include <ddspipe_participants/participant/rtps/InitialPeersParticipant.hpp>
#include <ddspipe_participants/participant/rtps/SimpleParticipant.hpp>
#include <ddspipe_participants/participant/dynamic_types/DynTypesPublicationParticipant.hpp>
#include <ddspipe_participants/participant/dynamic_types/DynTypesSubscriptionParticipant.hpp>

#include <ddsrouter_core/types/ParticipantKind.hpp>
#include <ddsrouter_core/core/ParticipantFactory.hpp>

namespace eprosima {
namespace ddsrouter {
namespace core {

template <typename ConfigurationType>
std::shared_ptr<ConfigurationType> convert_configuration_(
        const types::ParticipantKind& kind,
        const std::shared_ptr<ddspipe::participants::ParticipantConfiguration>& participant_configuration)
{
    std::shared_ptr<ConfigurationType> conf_ =
            std::dynamic_pointer_cast<ConfigurationType>(
        participant_configuration);
    if (!conf_)
    {
        throw utils::ConfigurationException(
                    utils::Formatter() << "Configuration from Participant: " << participant_configuration->id <<
                        " is not for Participant Kind: " << kind);
    }
    return conf_;
}

template <typename ConfigurationType, typename ParticipantType>
std::shared_ptr<ParticipantType> create_participant_no_init_(
        const types::ParticipantKind& kind,
        const std::shared_ptr<ddspipe::participants::ParticipantConfiguration>& participant_configuration,
        const std::shared_ptr<ddspipe::core::DiscoveryDatabase>& discovery_database)
{
    return std::make_shared<ParticipantType> (
        convert_configuration_<ConfigurationType>(kind, participant_configuration),
        discovery_database);
}

template <typename ConfigurationType, typename ParticipantType>
std::shared_ptr<ParticipantType> create_participant_no_init_(
        const types::ParticipantKind& kind,
        const std::shared_ptr<ddspipe::participants::ParticipantConfiguration>& participant_configuration,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        const std::shared_ptr<ddspipe::core::DiscoveryDatabase>& discovery_database)
{
    return std::make_shared<ParticipantType> (
        convert_configuration_<ConfigurationType>(kind, participant_configuration),
        payload_pool,
        discovery_database);
}

template <typename ConfigurationType, typename ParticipantType>
std::shared_ptr<ddspipe::core::IParticipant> create_participant_(
        const types::ParticipantKind& kind,
        const std::shared_ptr<ddspipe::participants::ParticipantConfiguration>& participant_configuration,
        const std::shared_ptr<ddspipe::core::DiscoveryDatabase>& discovery_database)
{
    auto participant = create_participant_no_init_<ConfigurationType, ParticipantType>(kind, participant_configuration, discovery_database);
    participant->init();
    return participant;
}

template <typename ConfigurationType, typename ParticipantType>
std::shared_ptr<ddspipe::core::IParticipant> create_participant_(
        const types::ParticipantKind& kind,
        const std::shared_ptr<ddspipe::participants::ParticipantConfiguration>& participant_configuration,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        const std::shared_ptr<ddspipe::core::DiscoveryDatabase>& discovery_database)
{
    auto participant = create_participant_no_init_<ConfigurationType, ParticipantType>(kind, participant_configuration, payload_pool, discovery_database);
    participant->init();
    return participant;
}

std::shared_ptr<ddspipe::core::IParticipant> ParticipantFactory::create_participant(
        const types::ParticipantKind& kind,
        const std::shared_ptr<ddspipe::participants::ParticipantConfiguration>& participant_configuration,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        const std::shared_ptr<ddspipe::core::DiscoveryDatabase>& discovery_database)
{
    // Create a new Participant depending on the ParticipantKind specified by the configuration
    switch (kind)
    {
        case types::ParticipantKind::echo:
            // Echo Participant
        {
            return create_participant_no_init_<
                ddspipe::participants::EchoParticipantConfiguration,
                ddspipe::participants::EchoParticipant>(
                    kind,
                    participant_configuration,
                    discovery_database
                );
        }

        case types::ParticipantKind::simple:
            // Simple RTPS Participant
        {
            return create_participant_<
                ddspipe::participants::SimpleParticipantConfiguration,
                ddspipe::participants::rtps::SimpleParticipant>(
                    kind,
                    participant_configuration,
                    payload_pool,
                    discovery_database
                );
        }

        case types::ParticipantKind::discovery_server:
            // Discovery Server RTPS Participant
        {
            return create_participant_<
                ddspipe::participants::DiscoveryServerParticipantConfiguration,
                ddspipe::participants::rtps::DiscoveryServerParticipant>(
                    kind,
                    participant_configuration,
                    payload_pool,
                    discovery_database
                );
        }

        case types::ParticipantKind::initial_peers:
            // Initial Peers RTPS Participant
        {
            return create_participant_<
                ddspipe::participants::InitialPeersParticipantConfiguration,
                ddspipe::participants::rtps::InitialPeersParticipant>(
                    kind,
                    participant_configuration,
                    payload_pool,
                    discovery_database
                );
        }

        case types::ParticipantKind::dyn_types_publication:
            // Dynamic Types Publication Participant
        {
            return create_participant_<
                ddspipe::participants::SimpleParticipantConfiguration,
                ddspipe::participants::DynTypesPublicationParticipant>(
                    kind,
                    participant_configuration,
                    discovery_database
                );
        }

        case types::ParticipantKind::dyn_types_subscription:
            // Dynamic Types Subscription Participant
        {
            return create_participant_<
                ddspipe::participants::SimpleParticipantConfiguration,
                ddspipe::participants::DynTypesSubscriptionParticipant>(
                    kind,
                    participant_configuration,
                    discovery_database
                );
        }

        default:
            // This should not happen as every kind must be in the switch
            utils::tsnh(
                utils::Formatter() << "Value of ParticipantKind out of enumeration.");
            return nullptr; // Unreachable code
    }
}

} /* namespace core */
} /* namespace ddsrouter */
} /* namespace eprosima */
