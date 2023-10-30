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
// limitations under the License.

#include <ddspipe_core/types/dynamic_types/types.hpp>

#include <ddspipe_participants/configuration/DiscoveryServerParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/EchoParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/InitialPeersParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/ParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/SimpleParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/XmlParticipantConfiguration.hpp>

#include <ddspipe_yaml/yaml_configuration_tags.hpp>
#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlManager.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddspipe_core/configuration/DdsPipeConfiguration.hpp>
#include <ddsrouter_core/configuration/DdsRouterConfiguration.hpp>

#include <ddsrouter_yaml/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddspipe {
namespace yaml {

template <>
void YamlReader::fill(
        ddsrouter::core::SpecsConfiguration& object,
        const Yaml& yml,
        const YamlReaderVersion version)
{
    /////
    // Get optional number of threads
    if (YamlReader::is_tag_present(yml, NUMBER_THREADS_TAG))
    {
        object.number_of_threads = YamlReader::get<unsigned int>(yml, NUMBER_THREADS_TAG, version);
    }

    /////
    // Get optional maximum history depth
    if (YamlReader::is_tag_present(yml, MAX_HISTORY_DEPTH_TAG))
    {
        object.max_history_depth = YamlReader::get<unsigned int>(yml, MAX_HISTORY_DEPTH_TAG, version);
    }

    /////
    // Get optional remove unused entities tag
    if (YamlReader::is_tag_present(yml, REMOVE_UNUSED_ENTITIES_TAG))
    {
        object.remove_unused_entities = YamlReader::get<bool>(yml, REMOVE_UNUSED_ENTITIES_TAG, version);
    }

    /////
    // Get optional max transmission rate
    if (YamlReader::is_tag_present(yml, MAX_TX_RATE_TAG))
    {
        auto max_tx_rate = YamlReader::get_nonnegative_float(yml, MAX_TX_RATE_TAG);

        // Save the max reception rate value in the advanced options
        object.max_tx_rate = max_tx_rate;

        // Set default value for max reception rate
        ddspipe::core::types::TopicQoS::default_max_tx_rate.store(max_tx_rate);
    }

    /////
    // Get optional max reception rate
    if (YamlReader::is_tag_present(yml, MAX_RX_RATE_TAG))
    {
        auto max_rx_rate = YamlReader::get_nonnegative_float(yml, MAX_RX_RATE_TAG);

        // Save the max reception rate value in the advanced options
        object.max_rx_rate = max_rx_rate;

        // Set default value for max reception rate
        ddspipe::core::types::TopicQoS::default_max_rx_rate.store(max_rx_rate);
    }

    // Get optional downsampling
    if (YamlReader::is_tag_present(yml, DOWNSAMPLING_TAG))
    {
        auto downsampling = YamlReader::get_positive_int(yml, DOWNSAMPLING_TAG);

        // Save the downsampling value in the advanced options
        object.downsampling = downsampling;

        // Set default value for downsampling
        ddspipe::core::types::TopicQoS::default_downsampling.store(downsampling);
    }
}

template <>
ddsrouter::core::types::ParticipantKind YamlReader::get(
        const Yaml& yml,
        const YamlReaderVersion /* version */)
{
    // Domain id required
    return get_enumeration_from_builder<ddsrouter::core::types::ParticipantKind>(yml,
                   *ddsrouter::core::types::ParticipantKindBuilder::get_instance());
}

template <>
std::shared_ptr<participants::ParticipantConfiguration>
YamlReader::get<std::shared_ptr<participants::ParticipantConfiguration>>(
        const Yaml& yml,
        const YamlReaderVersion version)
{
    // Kind required
    ddsrouter::core::types::ParticipantKind kind = YamlReader::get<ddsrouter::core::types::ParticipantKind>(yml,
                    PARTICIPANT_KIND_TAG,
                    version);

    logInfo(DDSROUTER_YAML_CONFIGURATION, "Loading Participant of kind " << kind << ".");

    switch (kind)
    {
        case ddsrouter::core::types::ParticipantKind::echo:
            return std::make_shared<participants::EchoParticipantConfiguration>(
                YamlReader::get<participants::EchoParticipantConfiguration>(yml, version));

        case ddsrouter::core::types::ParticipantKind::simple:
        case ddsrouter::core::types::ParticipantKind::dyn_types_subscription:
        case ddsrouter::core::types::ParticipantKind::dyn_types_publication:
            return std::make_shared<participants::SimpleParticipantConfiguration>(
                YamlReader::get<participants::SimpleParticipantConfiguration>(yml, version));

        case ddsrouter::core::types::ParticipantKind::discovery_server:
            return std::make_shared<participants::DiscoveryServerParticipantConfiguration>(
                YamlReader::get<participants::DiscoveryServerParticipantConfiguration>(yml, version));

        case ddsrouter::core::types::ParticipantKind::initial_peers:
            return std::make_shared<participants::InitialPeersParticipantConfiguration>(
                YamlReader::get<participants::InitialPeersParticipantConfiguration>(yml, version));

        case ddsrouter::core::types::ParticipantKind::xml:
            return std::make_shared<participants::XmlParticipantConfiguration>(
                YamlReader::get<participants::XmlParticipantConfiguration>(yml, version));

        default:
            // Non recheable code
            throw eprosima::utils::ConfigurationException(
                      utils::Formatter() << "Unkown or non valid Participant kind: " << kind << ".");
    }
}

template <>
void YamlReader::fill(
        ddspipe::core::DdsPipeConfiguration& object,
        const Yaml& yml,
        const YamlReaderVersion version)
{
    /////
    // Get optional allowlist
    if (YamlReader::is_tag_present(yml, ALLOWLIST_TAG))
    {
        auto allowlist_set = YamlReader::get_set<core::types::WildcardDdsFilterTopic>(yml, ALLOWLIST_TAG, version);
        for (auto const& wild_topic : allowlist_set)
        {
            auto new_topic = utils::Heritable<core::types::WildcardDdsFilterTopic>::make_heritable(wild_topic);
            object.allowlist.insert(new_topic);
        }

        // Add to allowlist always the type object topic
        core::types::WildcardDdsFilterTopic internal_topic;
        internal_topic.topic_name.set_value(core::types::TYPE_OBJECT_TOPIC_NAME);
        object.allowlist.insert(
            utils::Heritable<core::types::WildcardDdsFilterTopic>::make_heritable(internal_topic));
    }

    /////
    // Get optional blocklist
    if (YamlReader::is_tag_present(yml, BLOCKLIST_TAG))
    {
        auto blocklist_set = YamlReader::get_set<core::types::WildcardDdsFilterTopic>(yml, BLOCKLIST_TAG, version);
        for (auto const& wild_topic : blocklist_set)
        {
            auto new_topic = utils::Heritable<core::types::WildcardDdsFilterTopic>::make_heritable(wild_topic);
            object.blocklist.insert(new_topic);
        }
    }

    /////
    // Get optional builtin topics
    if (YamlReader::is_tag_present(yml, BUILTIN_TAG))
    {
        object.builtin_topics = YamlReader::get_set<utils::Heritable<ddspipe::core::types::DistributedTopic>>(yml,
                        BUILTIN_TAG,
                        version);
    }

    /////
    // Get optional routes
    if (YamlReader::is_tag_present(yml, ROUTES_TAG))
    {
        YamlReader::fill<ddspipe::core::RoutesConfiguration>(
            object.routes,
            YamlReader::get_value_in_tag(yml, ROUTES_TAG),
            version);
    }

    /////
    // Get optional topic routes
    if (YamlReader::is_tag_present(yml, TOPIC_ROUTES_TAG))
    {
        // get list, and parse each element as above
        YamlReader::fill<ddspipe::core::TopicRoutesConfiguration>(
            object.topic_routes,
            YamlReader::get_value_in_tag(yml, TOPIC_ROUTES_TAG),
            version);
    }

    /////
    // Get optional topics
    if (YamlReader::is_tag_present(yml, TOPICS_TAG))
    {
        auto manual_topics = YamlReader::get_list<core::types::WildcardDdsFilterTopic>(yml, TOPICS_TAG, version);

        for (auto const& manual_topic : manual_topics)
        {
            auto new_topic = utils::Heritable<core::types::WildcardDdsFilterTopic>::make_heritable(manual_topic);
            object.manual_topics.push_back(new_topic);
        }
    }
}

template <>
ddspipe::core::DdsPipeConfiguration YamlReader::get<ddspipe::core::DdsPipeConfiguration>(
        const Yaml& yml,
        const YamlReaderVersion version)
{
    ddspipe::core::DdsPipeConfiguration object;
    fill<ddspipe::core::DdsPipeConfiguration>(object, yml, version);
    return object;
}

template <>
void YamlReader::fill(
        ddsrouter::core::DdsRouterConfiguration& object,
        const Yaml& yml,
        const YamlReaderVersion version)
{
    /////
    // Get participants configurations. Required field, if get_value_in_tag fail propagate exception.
    auto participants_configurations_yml = YamlReader::get_value_in_tag(yml, COLLECTION_PARTICIPANTS_TAG);

    // TODO do it in a single instruction
    // Check it is a list
    if (!participants_configurations_yml.IsSequence())
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() <<
                      "Participant configurations must be specified in an array under tag: " <<
                      COLLECTION_PARTICIPANTS_TAG);
    }

    for (auto conf : participants_configurations_yml)
    {
        ddsrouter::core::types::ParticipantKind kind =
                YamlReader::get<ddsrouter::core::types::ParticipantKind>(conf, PARTICIPANT_KIND_TAG, version);
        object.participants_configurations.insert(
                    {
                        kind,
                        YamlReader::get<std::shared_ptr<participants::ParticipantConfiguration>>(conf, version)
                    }
            );
    }

    /////
    // Get optional specs configuration
    if (YamlReader::is_tag_present(yml, SPECS_TAG))
    {
        YamlReader::fill<ddsrouter::core::SpecsConfiguration>(
            object.advanced_options,
            YamlReader::get_value_in_tag(yml, SPECS_TAG),
            version);
    }

    // DDS Pipe Configuration
    object.ddspipe_configuration = YamlReader::get<ddspipe::core::DdsPipeConfiguration>(yml, version);

    /* NOTE
     *
     * remove_unused_entities is an attribute of SpecsConfiguration because it is under the tag specs,
     * but since it is used in the DdsPipe, we have two choices: copying it to the DdsPipeConfiguration,
     * as we are doing, or refill the SpecsConfiguraton in the DdsPipeConfiguration fill and take it from there.
     */
    object.ddspipe_configuration.remove_unused_entities = object.advanced_options.remove_unused_entities;

    /////
    // Get optional xml configuration
    if (YamlReader::is_tag_present(yml, XML_TAG))
    {
        YamlReader::fill<ddspipe::participants::XmlHandlerConfiguration>(
            object.xml_configuration,
            YamlReader::get_value_in_tag(yml, XML_TAG),
            version);
    }
}

template <>
ddsrouter::core::DdsRouterConfiguration YamlReader::get<ddsrouter::core::DdsRouterConfiguration>(
        const Yaml& yml,
        const YamlReaderVersion version)
{
    ddsrouter::core::DdsRouterConfiguration object;
    fill<ddsrouter::core::DdsRouterConfiguration>(object, yml, version);
    return object;
}

} // namespace yaml
} // namespace ddspipe
} // namespace eprosima
