// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "idl/TypesPubSubTypes.h"
#include "idl/TypesTypeObject.h"

#include <thread>

#include "TestPublisher.h"
#include "TestSubscriber.h"

#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/log/Log.h>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <thread>
#include <memory>
#include <cstdlib>
#include <string>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

static int DOMAIN = 10;

// TODO - Remove DISABLED when XTYPES are implemented.

class xtypes : public ::testing::Test
{
public:

    xtypes()
    {
        //Log::SetVerbosity(Log::Info);
        //Log::SetCategoryFilter(std::regex("(SECURITY)"));
        registerTypesTypes();
    }

    ~xtypes()
    {
        //Log::Reset();
        Log::KillThread();
        eprosima::fastrtps::Domain::stopAll();
        ++DOMAIN;
    }
};

/**** NO TYPE OBJECT ****/

/*
 * Both endpoints share the same type without DataRepresentationQos, so they must match.
 * This represents classical match.
*/
TEST_F(xtypes, NoTypeObjectSameType)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    pub.init("NoTypeObjectSameType", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("NoTypeObjectSameType", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", nullptr, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints share the same type without DataRepresentationQosm, but they force type validation,
 * so they must not match.
*/
TEST_F(xtypes, NoTypeObjectSameTypeForce)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    typeConQos.m_force_type_validation = true;

    pub.init("NoTypeObjectSameTypeForce", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("NoTypeObjectSameTypeForce", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", nullptr, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * The endpoints have different types without DataRepresentationQos, so they must not match.
 * This represents classical mismatch.
*/
TEST_F(xtypes, NoTypeObjectDifferentType)
{
    TypeSupport type(new BasicStructPubSubType());
    TypeSupport type2(new BasicNamesStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    pub.init("NoTypeObjectDifferentType", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("NoTypeObjectDifferentType", DOMAIN, NO_KEY, type2, nullptr, nullptr, nullptr, "Sub1", nullptr, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/**** TYPE OBJECT V1 ****/

/*
 * Both endpoints share the same type using XCDR1 so they must match.
 * This represents classical match.
*/
TEST_F(xtypes, TypeObjectV1SameType)
{
    TypeSupport type(new BasicStructPubSubType());
    const TypeObject* type_obj = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id = GetBasicStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    //TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1SameType", DOMAIN, type, type_obj, type_id, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1SameType", DOMAIN, NO_KEY, type, type_obj, type_id, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 30);
    pub.waitDiscovery(true, 30);
}

/*
 * TEST TYPE DISCOVERY
 * RETRIEVE PUB's TYPE
*/
TEST_F(xtypes, TypeDiscoverySubs)
{
    TypeSupport type(new BasicStructPubSubType());
    const TypeObject* type_obj = GetCompleteBasicStructObject();
    const TypeIdentifier* type_id = GetBasicStructIdentifier(true);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    //TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeDiscoverySubs", DOMAIN, type, type_obj, type_id, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeDiscoverySubs", DOMAIN, NO_KEY, TypeSupport(nullptr), nullptr, nullptr, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitTypeDiscovery(true, 3);
    //pub.waitDiscovery(true, 3);

    types::DynamicType_ptr disc_type = sub.discovered_type();
    ASSERT_TRUE(disc_type != nullptr);

    sub.register_discovered_type();
    sub.create_datareader();

    pub.waitDiscovery(true, 3);
    sub.waitDiscovery(true, 3);

    //reader->set_listener(nullptr);
    //sub.delete_datareader(reader);
}

/*
 * TEST TYPE DISCOVERY
 * RETRIEVE SUB's TYPE
*/
TEST_F(xtypes, TypeDiscoveryPubs)
{
    TypeSupport type(new BasicStructPubSubType());
    const TypeObject* type_obj = GetCompleteBasicStructObject();
    const TypeIdentifier* type_id = GetBasicStructIdentifier(true);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    //TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeDiscoveryPubs", DOMAIN, TypeSupport(nullptr), nullptr, nullptr, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeDiscoveryPubs", DOMAIN, NO_KEY, type, type_obj, type_id, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    pub.waitTypeDiscovery(true, 3);
    //pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints different types using XCDR1 so they must not match even with full coercion allowed.
*/
TEST_F(xtypes, TypeObjectV1DifferentType)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicBadStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicBadStructObject();
    const TypeIdentifier* type_id2 = GetBasicBadStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    typeConQos.m_ignore_sequence_bounds = true;
    typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1DifferentType", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1DifferentType", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in members.
 * This test checks failure at the member names through TypeIdentifier's hashes. The endpoints must not match.
*/
TEST_F(xtypes, TypeObjectV1NamesManaged)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicNamesStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicNamesStructObject();
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = false;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1NamesManaged", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1NamesManaged", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in struct and members.
 * This test checks ignoring at the member names through TypeIdentifier's hashes. The endpoints must match.
*/
TEST_F(xtypes, DISABLED_TypeObjectV1NamesIgnored)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicNamesStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicNamesStructObject();
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1NamesIgnored", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1NamesIgnored", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in struct and members.
 * This test checks ignoring at the member names when disallowed type coercion has no effect.
 * The endpoints must not match.
*/
TEST_F(xtypes, TypeObjectV1NamesIgnoredDisallow)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicNamesStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicNamesStructObject();
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1NamesIgnoredDisallow", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1NamesIgnoredDisallow", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicWideStruct are similar structures, but with "Wide" adds a member.
 * This test checks type widening. The endpoints must match.
*/
TEST_F(xtypes, DISABLED_TypeObjectV1TypeWidening)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicWideStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicWideStructObject();
    const TypeIdentifier* type_id2 = GetBasicWideStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1TypeWidening", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1TypeWidening", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * BasicStruct and BadBasicWideStruct are similar structures, but with "Wide" adds a member, and in this case
 * modifies the type of other member. The endpoints must not match.
*/
TEST_F(xtypes, TypeObjectV1BadTypeWidening)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BadBasicWideStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBadBasicWideStructObject();
    const TypeIdentifier* type_id2 = GetBadBasicWideStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1BadTypeWidening", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1BadTypeWidening", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicWideStruct are similar structures, but with "Wide" adds a member.
 * This test checks prevent type widening when assigning narrow to wide. The endpoints must match.
*/
TEST_F(xtypes, DISABLED_TypeObjectV1TypeWideningPreventedNarrowToWide)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicWideStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicWideStructObject();
    const TypeIdentifier* type_id2 = GetBasicWideStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = true;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1TypeWideningPreventedNarrowToWide", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1TypeWideningPreventedNarrowToWide", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * BasicStruct and BasicWideStruct are similar structures, but with "Wide" adds a member.
 * This test checks prevent type widening when assining from wide to narrow. The endpoints must not match.
*/
TEST_F(xtypes, TypeObjectV1TypeWideningPreventedWideToNarrow)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicWideStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicWideStructObject();
    const TypeIdentifier* type_id2 = GetBasicWideStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = true;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1TypeWideningPreventedWideToNarrow", DOMAIN, type2, type_obj2, type_id2, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1TypeWideningPreventedWideToNarrow", DOMAIN, NO_KEY, type1, type_obj1, type_id1, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicWideStruct are similar structures, but with "Wide" adds a member, but coercion is disallowed.
 * This test checks type widening. The endpoints must match.
*/
TEST_F(xtypes, TypeObjectV1TypeWideningDisallow)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicWideStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicWideStructObject();
    const TypeIdentifier* type_id2 = GetBasicWideStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1TypeWideningDisallow", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1TypeWideningDisallow", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * SequenceStruct and SequenceBoundsStruct are similar structures, but with "Bounds" the size of the sequence is bigger.
 * This test checks sequence bounds. The endpoints must match.
*/
TEST_F(xtypes, DISABLED_TypeObjectV1SequenceBoundsIgnored)
{
    TypeSupport type1(new SequenceStructPubSubType());
    TypeSupport type2(new SequenceBoundsStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalSequenceStructObject();
    const TypeIdentifier* type_id1 = GetSequenceStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalSequenceBoundsStructObject();
    const TypeIdentifier* type_id2 = GetSequenceBoundsStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1SequenceBoundsIgnored", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1SequenceBoundsIgnored", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * SequenceStruct and SequenceBoundsStruct are similar structures, but with "Bounds" the size of the sequence is bigger.
 * This test checks sequence bounds. The endpoints must not match.
*/
TEST_F(xtypes, TypeObjectV1SequenceBoundsManaged)
{
    TypeSupport type1(new SequenceStructPubSubType());
    TypeSupport type2(new SequenceBoundsStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalSequenceStructObject();
    const TypeIdentifier* type_id1 = GetSequenceStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalSequenceBoundsStructObject();
    const TypeIdentifier* type_id2 = GetSequenceBoundsStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    typeConQos.m_ignore_sequence_bounds = false;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1SequenceBoundsManaged", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1SequenceBoundsManaged", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * StringStruct and LargeStringStruct are similar structures, but with "Large" the size of the string is bigger.
 * Names are ignored because the member is named different.
 * This test checks string bounds. The endpoints must match.
*/
TEST_F(xtypes, DISABLED_TypeObjectV1LargeStringIgnored)
{
    TypeSupport type1(new StringStructPubSubType());
    TypeSupport type2(new LargeStringStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalStringStructObject();
    const TypeIdentifier* type_id1 = GetStringStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalLargeStringStructObject();
    const TypeIdentifier* type_id2 = GetLargeStringStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1LargeStringIgnored", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1LargeStringIgnored", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * StringStruct and LargeStringStruct are similar structures, but with "Large" the size of the string is bigger.
 * Names are ignored because the member is named different.
 * This test checks string bounds. The endpoints must not match.
*/
TEST_F(xtypes, TypeObjectV1LargeStringManaged)
{
    TypeSupport type1(new StringStructPubSubType());
    TypeSupport type2(new LargeStringStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalStringStructObject();
    const TypeIdentifier* type_id1 = GetStringStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalLargeStringStructObject();
    const TypeIdentifier* type_id2 = GetLargeStringStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = false;
    typeConQos.m_ignore_string_bounds = false;
    typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1LargeStringManaged", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1LargeStringManaged", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * Both endpoints share the same type using XCDR1 so they must match, in this case using Complete.
*/
TEST_F(xtypes, TypeObjectV1SameTypeComplete)
{
    TypeSupport type(new BasicStructPubSubType());
    const TypeObject* type_obj = GetCompleteBasicStructObject();
    const TypeIdentifier* type_id = GetBasicStructIdentifier(true);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    //TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1SameTypeComplete", DOMAIN, type, type_obj, type_id, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1SameTypeComplete", DOMAIN, NO_KEY, type, type_obj, type_id, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints use compatible but different types using XCDR1 but without coercion so they must not match,
 * in this case using Complete.
*/
TEST_F(xtypes, TypeObjectV1InvalidComplete)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicNamesStructPubSubType());
    const TypeObject* type_obj1 = GetCompleteBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(true);
    const TypeObject* type_obj2 = GetCompleteBasicNamesStructObject();
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(true);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    //TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1InvalidComplete", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1InvalidComplete", DOMAIN, NO_KEY, type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * Both endpoints the same type using XCDR1 but one using minimal and the other complete.
 * They shouldn't match.
*/
TEST_F(xtypes, MixingMinimalAndComplete)
{
    TypeSupport type1(new BasicStructPubSubType());
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetCompleteBasicStructObject();
    const TypeIdentifier* type_id2 = GetBasicStructIdentifier(true);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    //TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("MixingMinimalAndComplete", DOMAIN, type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("MixingMinimalAndComplete", DOMAIN, NO_KEY, type1, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/**** TYPE IDENTIFIER ****/

/*
 * Both endpoints share the same type using XCDR1 so they must match.
 * This represents classical match.
*/
TEST_F(xtypes, TypeIdentifierSameType)
{
    TypeSupport type(new BasicStructPubSubType());
    const TypeIdentifier* type_id = GetBasicStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    //TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeIdentifierSameType", DOMAIN, type, nullptr, type_id, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeIdentifierSameType", DOMAIN, NO_KEY, type, nullptr, type_id, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints different types using XCDR1 so they must not match even with full coercion allowed.
*/
TEST_F(xtypes, TypeIdentifierDifferentType)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicBadStructPubSubType());
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeIdentifier* type_id2 = GetBasicBadStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    typeConQos.m_ignore_sequence_bounds = true;
    typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    typeConQos.m_force_type_validation = false;

    pub.init("TypeIdentifierDifferentType", DOMAIN, type1, nullptr, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeIdentifierDifferentType", DOMAIN, NO_KEY, type2, nullptr, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in members.
 * This test checks failure at the member names through TypeIdentifier's hashes. The endpoints must not match.
*/
TEST_F(xtypes, TypeIdentifierNamesManaged)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicNamesStructPubSubType());
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = false;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeIdentifierNamesManaged", DOMAIN, type1, nullptr, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeIdentifierNamesManaged", DOMAIN, NO_KEY, type2, nullptr, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in struct and members.
 * This test checks ignoring at the member names through TypeIdentifier's hashes. The endpoints must match.
*/
TEST_F(xtypes, DISABLED_TypeIdentifierNamesIgnored)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicNamesStructPubSubType());
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeIdentifierNamesIgnored", DOMAIN, type1, nullptr, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeIdentifierNamesIgnored", DOMAIN, NO_KEY, type2, nullptr, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/**** TYPE INFORMATION ****/

/*
 * Both endpoints share the same type using XCDR1 so they must match.
 * This represents classical match.
*/
TEST_F(xtypes, TypeInformationSameType)
{
    TypeSupport type(new BasicStructPubSubType());
    const TypeInformation* type_info = TypeObjectFactory::get_instance()->get_type_information("BasicStruct");
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    //TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeInformationSameType", DOMAIN, type, nullptr, nullptr, type_info, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeInformationSameType", DOMAIN, NO_KEY, type, nullptr, nullptr, type_info, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints different types using XCDR1 so they must not match even with full coercion allowed.
*/
TEST_F(xtypes, TypeInformationDifferentType)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicBadStructPubSubType());
    const TypeInformation* type_info1 = TypeObjectFactory::get_instance()->get_type_information("BasicStruct");
    const TypeInformation* type_info2 = TypeObjectFactory::get_instance()->get_type_information("BasicBadStruct");
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    typeConQos.m_ignore_sequence_bounds = true;
    typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    typeConQos.m_force_type_validation = false;

    pub.init("TypeInformationDifferentType", DOMAIN, type1, nullptr, nullptr, type_info1, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeInformationDifferentType", DOMAIN, NO_KEY, type2, nullptr, nullptr, type_info2, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in members.
 * This test checks failure at the member names through TypeInformation's hashes. The endpoints must not match.
*/
TEST_F(xtypes, TypeInformationNamesManaged)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicNamesStructPubSubType());
    const TypeInformation* type_info1 = TypeObjectFactory::get_instance()->get_type_information("BasicStruct");
    const TypeInformation* type_info2 = TypeObjectFactory::get_instance()->get_type_information("BasicNamesStruct");
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = false;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeInformationNamesManaged", DOMAIN, type1, nullptr, nullptr, type_info1, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeInformationNamesManaged", DOMAIN, NO_KEY, type2, nullptr, nullptr, type_info2, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in struct and members.
 * This test checks ignoring at the member names through TypeInformation's hashes. The endpoints must match.
*/
TEST_F(xtypes, DISABLED_TypeInformationNamesIgnored)
{
    TypeSupport type1(new BasicStructPubSubType());
    TypeSupport type2(new BasicNamesStructPubSubType());
    const TypeInformation* type_info1 = TypeObjectFactory::get_instance()->get_type_information("BasicStruct");
    const TypeInformation* type_info2 = TypeObjectFactory::get_instance()->get_type_information("BasicNamesStruct");
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeInformationNamesIgnored", DOMAIN, type1, nullptr, nullptr, type_info1, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeInformationNamesIgnored", DOMAIN, NO_KEY, type2, nullptr, nullptr, type_info2, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/**** Mixing TypeObject, TypeInformation and TypeIdentifier ****/

TEST_F(xtypes, TypeIdentifier_TypeObject)
{
    TypeSupport type(new BasicStructPubSubType());
    const TypeObject* type_obj = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id = GetBasicStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    typeConQos.m_force_type_validation = true;

    pub.init("TypeIdentifier_TypeObject", DOMAIN, type, nullptr, type_id, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeIdentifier_TypeObject", DOMAIN, NO_KEY, type, type_obj, nullptr, nullptr, "Sub1", nullptr, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

TEST_F(xtypes, TypeIdentifier_TypeInformation)
{
    TypeSupport type(new BasicStructPubSubType());
    const TypeIdentifier* type_id = GetBasicStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    const TypeInformation* type_info = TypeObjectFactory::get_instance()->get_type_information("BasicStruct");

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    typeConQos.m_force_type_validation = true;

    pub.init("TypeIdentifier_TypeInformation", DOMAIN, type, nullptr, type_id, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeIdentifier_TypeInformation", DOMAIN, NO_KEY, type, nullptr, nullptr, type_info, "Sub1", nullptr, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

TEST_F(xtypes, TypeObject_TypeInformation)
{
    TypeSupport type(new BasicStructPubSubType());
    const TypeObject* type_obj = GetMinimalBasicStructObject();
    TestPublisher pub;
    TestSubscriber sub;
    const TypeInformation* type_info = TypeObjectFactory::get_instance()->get_type_information("BasicStruct");

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    typeConQos.m_force_type_validation = true;

    pub.init("TypeObject_TypeInformation", DOMAIN, type, type_obj, nullptr, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObject_TypeInformation", DOMAIN, NO_KEY, type, nullptr, nullptr, type_info, "Sub1", nullptr, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/**** TODO - Enable when implemented - DataRepresentation Compatibility tests ****/

/*
 * Empty-Empty
*/
TEST_F(xtypes, DISABLED_DataRepQoSEE)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos;
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSEE", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSEE", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Empty-XCDR1
*/
TEST_F(xtypes, DISABLED_DataRepQoSE1)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSE1", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos1);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSE1", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Empty-XCDR2
*/
TEST_F(xtypes, DISABLED_DataRepQoSE2)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSE2", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos1);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSE2", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Empty-XML
*/
TEST_F(xtypes, DISABLED_DataRepQoSEX)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSEX", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos1);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSEX", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * Empty-XCDR12
*/
TEST_F(xtypes, DISABLED_DataRepQoSE12)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSE12", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos1);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSE12", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * XCDR1-XCDR2
*/
TEST_F(xtypes, DISABLED_DataRepQoS12)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoS12", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos1);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoS12", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * XCDR2-Empty
*/
TEST_F(xtypes, DISABLED_DataRepQoS2E)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoS21", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos2);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoS21", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos1, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * XCDR2-XCDR1
*/
TEST_F(xtypes, DISABLED_DataRepQoS21)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoS21", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos2);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoS21", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos1, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * XCDR2-XCDR2
*/
TEST_F(xtypes, DISABLED_DataRepQoS22)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoS22", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos2);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoS22", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * XML-Empty
*/
TEST_F(xtypes, DISABLED_DataRepQoSXE)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSXE", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos2);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSXE", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos1, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * XML-XML - Fails until supported.
*/
TEST_F(xtypes, DISABLED_DataRepQoSXX)
{
    TypeSupport type(new BasicStructPubSubType());
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSXE", DOMAIN, type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos2);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSXE", DOMAIN, NO_KEY, type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos1, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
