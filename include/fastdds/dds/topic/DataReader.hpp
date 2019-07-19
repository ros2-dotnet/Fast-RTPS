// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataReader.hpp
 *
 */

#ifndef _FASTRTPS_DATAREADER_HPP_
#define _FASTRTPS_DATAREADER_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastdds/rtps/common/Time_t.h>

#include <vector>
#include <cstdint>

namespace eprosima {
namespace fastrtps {

class TopicAttributes;
class ReaderQos;
class LivelinessChangedStatus;

namespace rtps
{
class RTPSReader;
class ReaderAttributes;
class GUID_t;
class InstanceHandle_t;
}

class SampleInfo_t;

} // namespace fastrtps

namespace fastdds {
namespace dds {

class Subscriber;
class SubscriberImpl;
class DataReaderImpl;
class DataReaderListener;
class TypeSupport;

/**
 * Class DataReader, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTDDS_MODULE
 */
class DataReader {
    friend class SubscriberImpl;

    /**
    * Creates a DataReader. Don't use it directly, but through Subscriber.
    */
    DataReader(
            DataReaderImpl* impl);

public:
    virtual ~DataReader();

    /**
     * Method to block the current thread until an unread message is available
     */
    bool wait_for_unread_message(
            const fastrtps::Duration_t& timeout);


    /** @name Read or take data methods.
     * Methods to read or take data from the History.
     */

    ///@{

    /* TODO
    bool read(
            std::vector<void*>& data_values,
            std::vector<fastrtps::SampleInfo_t>& sample_infos,
            uint32_t max_samples);
    */

    bool read_next_sample(
            void* data,
            fastrtps::SampleInfo_t* info);

    /* TODO
    bool take(
            std::vector<void*>& data_values,
            std::vector<fastrtps::SampleInfo_t>& sample_infos,
            uint32_t max_samples);
    */

    bool take_next_sample(
            void* data,
            fastrtps::SampleInfo_t* info);

    ///@}

    /**
    * Get associated GUID
    * @return Associated GUID
    */
    const fastrtps::rtps::GUID_t& guid();

    fastrtps::rtps::InstanceHandle_t get_instance_handle() const;

    /**
    * Get topic data type
    * @return Topic data type
    */
    TypeSupport type();

    /**
     * @brief Get the requested deadline missed status
     * @return The deadline missed status
     */
    void get_requested_deadline_missed_status(
            fastrtps::RequestedDeadlineMissedStatus& status);

    bool set_attributes(const fastrtps::rtps::ReaderAttributes& att);

    const fastrtps::rtps::ReaderAttributes& get_attributes() const;

    bool set_qos(const fastrtps::ReaderQos& qos);

    const fastrtps::ReaderQos& get_qos() const;

    bool set_topic(const fastrtps::TopicAttributes& att);

    const fastrtps::TopicAttributes& get_topic() const;

    bool set_listener(DataReaderListener* listener);

    const DataReaderListener* get_listener() const;

    /* TODO
    bool get_key_value(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);
    */

    bool get_liveliness_changed_status(
            fastrtps::LivelinessChangedStatus& status) const;

    /* TODO
    bool get_requested_incompatible_qos_status(
            fastrtps::RequestedIncompatibleQosStatus& status) const;
    */

    /* TODO
    bool get_sample_lost_status(
            fastrtps::SampleLostStatus& status) const;
    */

    /* TODO
    bool get_sample_rejected_status(
            fastrtps::SampleRejectedStatus& status) const;
    */

    const Subscriber* get_subscriber() const;

    /* TODO
    bool wait_for_historical_data(
            const fastrtps::Duration_t& max_wait) const;
    */

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    void disable();

private:
    DataReaderImpl* impl_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* _FASTRTPS_DATAREADER_HPP_*/
