
#include "gtfs-realtime.pb.h"
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    std::cout << "init:0" << std::endl;
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::cout << "init:1" << std::endl;

    transit_realtime::Alert alrt;
    transit_realtime::TripUpdate trpUpdate;
    transit_realtime::FeedEntity entity;
    transit_realtime::FeedMessage feedMessage;
    transit_realtime::VehiclePosition vehiclePosition;
    
    std::cout << "init:2" << std::endl;

    std::fstream testDataFile("C:/Users/berkh/Projects/Github_repo/PublicTransport-Analysis/dataset/realtime_historical_data/sl-TripUpdates-2025-01-22/sl/TripUpdates/2025/01/22/00/sl-tripupdates-2025-01-21T23-59-38Z.pb", std::ios::in | std::ios::binary);
    if (!testDataFile) {
        std::cout << "Failed to open file." << std::endl;
        return -1;
    }

    std::cout << "init:3" << std::endl;

    trpUpdate.ParseFromIstream(&testDataFile);

    std::cout << "init:4" << std::endl;

    std::cout << "TripUpdate: " << trpUpdate.DebugString() << std::endl;
    std::cout << "TripUpdate size: " << trpUpdate.ByteSizeLong() << std::endl;
    std::cout << "TripUpdate size: " << trpUpdate.ByteSize() << std::endl;
    std::cout << "TripUpdate description: " << trpUpdate.GetDescriptor()->DebugString() << std::endl;
    std::cout << "TripUpdate field count: " << trpUpdate.GetDescriptor()->field_count() << std::endl;
    std::cout << "TripUpdate field name: " << trpUpdate.GetDescriptor()->FindFieldByName("trip")->name() << std::endl;
    std::cout << "TripUpdate field number: " << trpUpdate.GetDescriptor()->FindFieldByName("trip")->number() << std::endl;
    std::cout << "TripUpdate StopTimeUpdate size: " << trpUpdate.stop_time_update_size() << std::endl;
    std::cout << "TripUpdate StopTimeUpdate field count: " << trpUpdate.stop_time_update(0).GetDescriptor()->field_count() << std::endl;
    std::cout << "TripUpdate StopTimeUpdate field name: " << trpUpdate.stop_time_update(0).GetDescriptor()->FindFieldByName("arrival")->name() << std::endl;
    std::cout << "TripUpdate StopTimeUpdate field number: " << trpUpdate.stop_time_update(0).GetDescriptor()->FindFieldByName("arrival")->number() << std::endl;

    testDataFile.close();
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
