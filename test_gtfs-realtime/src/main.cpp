
#include "gtfs-realtime.pb.h"

#include <chrono>
#include <thread>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::string defaultPD_file("C:/Users/berkh/Projects/Github_repo/PublicTransport-Analysis/dataset/realtime_historical_data/sl-ServiceAlerts-2025-01-22/sl/ServiceAlerts/2025/01/22/09/sl-servicealerts-2025-01-22T09-29-28Z.pb");

    transit_realtime::Alert alrt;
    transit_realtime::TripUpdate trpUpdate;
    transit_realtime::TripDescriptor trpDesc;
    transit_realtime::FeedEntity entity;
    transit_realtime::FeedMessage feedMessage;
    transit_realtime::VehiclePosition vehiclePosition;
    
    std::fstream testDataFile;
    if(argc > 1) {
        testDataFile.open(argv[1], std::ios::in | std::ios::binary);
        // ("C:/Users/berkh/Projects/Github_repo/PublicTransport-Analysis/dataset/realtime_historical_data/sl-TripUpdates-2025-01-22/sl/TripUpdates/2025/01/22/00/sl-tripupdates-2025-01-21T23-59-38Z.pb", std::ios::in | std::ios::binary)
    }
    else {
        // std::cerr << "no file given..." <<std::endl;
        // return -1;
        std::cout << "NOTE: using defaultPD_file." << std::endl;
        testDataFile.open(defaultPD_file, std::ios::in | std::ios::binary);
    }

    if (!testDataFile) {
        std::cerr << "Failed to open file." << std::endl;
        return -1;
    }


    trpUpdate.ParseFromIstream(&testDataFile);
    alrt.ParseFromIstream(&testDataFile);
    trpDesc.ParseFromIstream(&testDataFile);
    // entity.ParseFromIstream(&testDataFile);
    // feedMessage.ParseFromIstream(&testDataFile);
    vehiclePosition.ParseFromIstream(&testDataFile);



    // std::cout << "--- TripUpdate: " << trpUpdate.DebugString() << std::endl; system("pause");
    // std::cout << "--- TripUpdate size: " << trpUpdate.ByteSizeLong() << std::endl; system("pause");
    // std::cout << "--- TripUpdate size: " << trpUpdate.ByteSize() << std::endl; system("pause");
    // std::cout << "--- TripUpdate description: " << trpUpdate.GetDescriptor()->DebugString() << std::endl; system("pause");
    // std::cout << "--- TripUpdate field count: " << trpUpdate.GetDescriptor()->field_count() << std::endl; system("pause");
    // std::cout << "--- TripUpdate field name: " << trpUpdate.GetDescriptor()->FindFieldByName("trip")->name() << std::endl; system("pause");
    // std::cout << "--- TripUpdate field number: " << trpUpdate.GetDescriptor()->FindFieldByName("trip")->number() << std::endl; system("pause");
    // std::cout << "--- TripUpdate StopTimeUpdate size: " << trpUpdate.stop_time_update_size() << std::endl; system("pause");
    // std::cout << "--- TripUpdate StopTimeUpdate field count: " << trpUpdate.stop_time_update(0).GetDescriptor()->field_count() << std::endl; system("pause");
    // std::cout << "--- TripUpdate StopTimeUpdate field name: " << trpUpdate.stop_time_update(0).GetDescriptor()->FindFieldByName("arrival")->name() << std::endl; system("pause");
    // std::cout << "--- TripUpdate StopTimeUpdate field number: " << trpUpdate.stop_time_update(0).GetDescriptor()->FindFieldByName("arrival")->number() << std::endl; system("pause");
    // return 0;
    // std::cout << trpUpdate.stop_time_update(0).GetDescriptor() << std::endl;

    auto trp = trpUpdate.trip();
    std::cout << trp.trip_id() << ", "<<trp.route_id()<<": ["<<trp.start_date()<<" | "<<trp.start_time()<<"_"<<trp.direction_id()<<"]"<<trp.schedule_relationship() << std::endl;
    std::cout << "delay                : " << trpUpdate.delay()<<std::endl;
    std::cout << "stop time update size: " << trpUpdate.stop_time_update_size() << std::endl;
    std::cout << "stop_time_updates:-------------------" << std::endl;
    system("pause");
    for(size_t i=0; i<trpUpdate.stop_time_update_size(); i++) {
        auto& stu = trpUpdate.stop_time_update(i);
        std::cout << "i:"<<std::setw(3)<<i<<": ";
        std::cout << stu.stop_sequence() << " | ";
        std::cout << stu.stop_id() << " | ";
        std::cout << stu.stop_sequence() << " | ";
        std::cout << stu.departure_occupancy_status() << " | ";
        std::cout << stu.schedule_relationship() << " | ";
        std::cout << std::endl;
    }
    std::cout << "tripProperties:-------------------" << std::endl;
    system("pause");
    auto trpProp = trpUpdate.trip_properties();
    std::cout << "trip_id    : " << trpProp.trip_id() << std::endl;
    std::cout << "start_date : " << trpProp.start_date() << std::endl;
    std::cout << "start_time : " << trpProp.start_time() << std::endl;
    std::cout << "shape_id   : " << trpProp.shape_id() << std::endl;

    testDataFile.close();
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
