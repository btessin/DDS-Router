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

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <test_utils.hpp>

#include <ddsrouter_core/types/participant/ParticipantId.hpp>
#include <ddsrouter_core/types/dds/DomainId.hpp>
#include <ddsrouter_yaml/YamlReader.hpp>

#include "../../YamlConfigurationTestUtils.hpp"

using namespace eprosima::ddsrouter;
using namespace eprosima::ddsrouter::yaml;

/**
 * Test get Participant Configuration from yaml specifing domain
 *
 * POSITIVE CASES:
 * - specify domain
 *
 * NEGATIVE CASES:
 * - incorrect domain format
 */
TEST(YamlGetDiscoveryServerParticipantConfigurationTest, get_participant_domain)
{
    core::types::ParticipantName name = eprosima::ddsrouter::test::random_participant_name();
    core::types::ParticipantKind kind = core::types::ParticipantKind::local_discovery_server;
    core::types::GuidPrefix guid_prefix = eprosima::ddsrouter::test::random_guid_prefix();

    // specify domain
    {
        Yaml yml;
        Yaml yml_participant;

        // Add required fields
        yaml::test::participantname_to_yaml(yml_participant, name);
        yaml::test::participantkind_to_yaml(yml_participant, kind);
        yaml::test::discovery_server_guid_prefix_to_yaml(yml_participant, guid_prefix);

        // Add addresses
        Yaml yml_listening_addresses;
        Yaml yml_address;
        core::types::Address address = eprosima::ddsrouter::test::random_address();
        yaml::test::address_to_yaml(yml_address, address);
        yml_listening_addresses.push_back(yml_address);

        yml_participant[LISTENING_ADDRESSES_TAG] = yml_listening_addresses;

        // Add domain
        core::types::DomainId domain = eprosima::ddsrouter::test::random_domain();
        yaml::test::domain_to_yaml(yml_participant, domain);

        yml["participant"] = yml_participant;

        // Get configuration object from yaml
        core::configuration::DiscoveryServerParticipantConfiguration result =
                YamlReader::get<core::configuration::DiscoveryServerParticipantConfiguration>(yml, "participant",
                        LATEST);

        // Check result
        ASSERT_EQ(domain, result.domain());
    }

    // incorrect domain format
    {
        Yaml yml;
        Yaml yml_participant;

        // Add required fields
        yaml::test::participantname_to_yaml(yml_participant, name);
        yaml::test::participantkind_to_yaml(yml_participant, kind);
        yaml::test::discovery_server_guid_prefix_to_yaml(yml_participant, guid_prefix);

        // Add addresses
        Yaml yml_listening_addresses;
        Yaml yml_address;
        core::types::Address address = eprosima::ddsrouter::test::random_address();
        yaml::test::address_to_yaml(yml_address, address);
        yml_listening_addresses.push_back(yml_address);

        // Add incorrect domain
        yml_participant["domain"] = "DOMAIN";

        yml["participant"] = yml_participant;

        // Get configuration object from yaml and expect fail
        ASSERT_THROW(
            core::configuration::DiscoveryServerParticipantConfiguration result =
            YamlReader::get<core::configuration::DiscoveryServerParticipantConfiguration>(yml, "participant", LATEST),
            utils::ConfigurationException);
    }
}
