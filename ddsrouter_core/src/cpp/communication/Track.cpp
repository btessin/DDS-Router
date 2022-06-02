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
 * @file Track.cpp
 *
 */

#include <ddsrouter_utils/exception/UnsupportedException.hpp>
#include <ddsrouter_utils/Log.hpp>

#include <communication/Track.hpp>

namespace eprosima {
namespace ddsrouter {
namespace core {

using namespace eprosima::ddsrouter::core::types;

const unsigned int Track::MAX_MESSAGES_TRANSMIT_LOOP_ = 100;

Track::Track(
        const RealTopic& topic,
        ParticipantId reader_participant_id,
        std::shared_ptr<IReader> reader,
        std::map<ParticipantId, std::shared_ptr<IWriter>>&& writers,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<thread::SlotThreadPool> thread_pool,
        bool enable /* = false */) noexcept
    : reader_participant_id_(reader_participant_id)
    , topic_(topic)
    , reader_(reader)
    , writers_(writers)
    , payload_pool_(payload_pool)
    , enabled_(false)
    , exit_(false)
    , data_available_status_(DataAvailableStatus::NO_MORE_DATA)
    , thread_pool_(thread_pool)
    , transmit_task_id_(thread::new_unique_task_id())
{
    logDebug(DDSROUTER_TRACK, "Creating Track " << *this << ".");

    // Set this track to on_data_available lambda call
    reader_->set_on_data_available_callback(std::bind(&Track::data_available_, this));

    // Set slot in thread pool
    thread_pool_->slot(
            transmit_task_id_,
            std::bind(&Track::transmit_, this));

    if (enable)
    {
        // Activate Track
        this->enable();
    }
    logDebug(DDSROUTER_TRACK, "Track " << *this << " created.");
}

Track::~Track()
{
    logDebug(DDSROUTER_TRACK, "Destroying Track " << *this << ".");

    // Disable reader and writers
    disable();

    // Unset callback on the Reader (this is needed as Reader will live longer than Track)
    reader_->unset_on_data_available_callback();

    // It does need to guard the mutex to avoid notifying Track thread while it is checking variable condition
    // Set exit status and call transmit thread to awake and terminate. Then wait for it.
    exit_.store(true);

    logDebug(DDSROUTER_TRACK, "Track " << *this << " destroyed.");
}

void Track::enable() noexcept
{
    std::lock_guard<std::mutex> lock(track_mutex_);

    if (!enabled_)
    {
        logInfo(DDSROUTER_TRACK, "Enabling Track " << reader_participant_id_ << " for topic " << topic_ << ".");
        enabled_ = true;

        // Enabling writers
        for (auto& writer_it : writers_)
        {
            writer_it.second->enable();
        }

        // Enabling reader
        reader_->enable();

    }
}

void Track::disable() noexcept
{
    std::lock_guard<std::mutex> lock(track_mutex_);

    if (enabled_)
    {
        logInfo(DDSROUTER_TRACK, "Disabling Track " << reader_participant_id_ << " for topic " << topic_ << ".");

        // Do disable before stop in the mutex so the Track is forced to stop in next iteration
        enabled_ = false;
        {
            // Stop if there is a transmission in course till the data is sent
            std::unique_lock<std::mutex> lock(on_transmission_mutex_);
        }

        // Disabling Reader
        reader_->disable();

        // Disabling Writers
        for (auto& writer_it : writers_)
        {
            writer_it.second->disable();
        }
    }
}

bool Track::should_transmit_() noexcept
{
    return !exit_ && enabled_ && this->is_data_available_();
}

void Track::data_available_() noexcept
{
    // Only hear callback if it is enabled
    if (enabled_)
    {
        logDebug(DDSROUTER_TRACK, "Track " << *this << " has data ready to be sent.");

        // Lock data_available_status_mutex_ to avoid changing the status while it is being checked
        std::lock_guard<std::mutex> lock(data_available_status_mutex_);

        // This method will always be called from the Reader thread, so it is safe to set the status
        DataAvailableStatus current_status = data_available_status_.exchange(DataAvailableStatus::NEW_DATA_ARRIVED);

        if (current_status == DataAvailableStatus::NO_MORE_DATA)
        {
            // Only send the callback to thread pool if it was not running
            thread_pool_->emit(transmit_task_id_);
            logDebug(DDSROUTER_TRACK, "Track " << *this << " send callback to queue.");
        }
    }
}

bool Track::is_data_available_() const noexcept
{
    return data_available_status_ == DataAvailableStatus::NEW_DATA_ARRIVED ||
           data_available_status_ == DataAvailableStatus::TRANSMITTING_DATA;
}

void Track::transmit_() noexcept
{
    // Loop that ends if it should stop transmitting (should_transmit_nts_).
    // Called inside the loop so it is protected by a mutex that is freed in every iteration.

    // Lock Mutex on_transmition while a data is being transmitted
    // This prevents the Track to be disabled (and disable writers and readers) while sending a data
    // enab led_ will be set to false before taking the mutex, so the track will finish after current iteration
    std::unique_lock<std::mutex> lock(on_transmission_mutex_);

    // TODO: Count the times it loops to break it at some point if needed
    while(should_transmit_())
    {
        // It starts transmitting, so it sets the data available status as transmitting
        data_available_status_ = DataAvailableStatus::TRANSMITTING_DATA;

        // Get data received
        std::unique_ptr<DataReceived> data = std::make_unique<DataReceived>();
        utils::ReturnCode ret = reader_->take(data);

        if (ret == utils::ReturnCode::RETCODE_NO_DATA)
        {
            // Lock data_available_status_mutex_ to avoid changing the status while it is being checked
            std::lock_guard<std::mutex> lock(data_available_status_mutex_);

            // There is no more data, so finish loop and wait again for new data
            DataAvailableStatus current_status = data_available_status_.exchange(DataAvailableStatus::NO_MORE_DATA);
            if (current_status == DataAvailableStatus::NEW_DATA_ARRIVED)
            {
                // New data has arrived while setting NO_MORE_DATA, so it should continues
                data_available_status_.store(DataAvailableStatus::TRANSMITTING_DATA);
                continue;
            }
            else
            {
                // NO_MORE_DATA has been set, so if any other data arrives it will send a callback to thread pool
                break;
            }
        }
        else if (!ret)
        {
            // Error reading data
            logWarning(DDSROUTER_TRACK, "Error taking data in Track " << topic_ << ". Error code " << ret
                                                                      << ". Skipping data and continue.");
            continue;
        }

        logDebug(DDSROUTER_TRACK,
                "Track " << reader_participant_id_ << " for topic " << topic_ <<
                " transmitting data from remote endpoint " << data->source_guid << ".");

        // Send data through writers
        for (auto& writer_it : writers_)
        {
            ret = writer_it.second->write(data);

            if (!ret)
            {
                logWarning(DDSROUTER_TRACK, "Error writting data in Track " << topic_ << ". Error code "
                                                                            << ret <<
                        ". Skipping data for this writer and continue.");
                continue;
            }
        }

        payload_pool_->release_payload(data->payload);
    }

    data_available_status_.store(DataAvailableStatus::NO_MORE_DATA);
}

std::ostream& operator <<(
        std::ostream& os,
        const Track& track)
{
    os << "Track{" << track.topic_ << ";" << track.reader_participant_id_ << "}";
    return os;
}

} /* namespace core */
} /* namespace ddsrouter */
} /* namespace eprosima */
