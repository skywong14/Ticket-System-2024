//
// Created by skywa on 2024/5/28.
//
#include "train.hpp"

Train_System::Train_System() {
    route_data.initialise("route_data");
    train_data.initialise("train_data");
    releasedTrain_data.initialise("releasedTrain_data");
    station_data.initialise("station_data");
}

Train_Route Train_System::read_Train_Route(int pos) {
    return route_data.read_T(pos);
}

void Train_System::update_Train_Route(int pos, Train_Route val_) {
    route_data.write_T(pos, val_);
}

int Train_System::new_Train_Route(Train_Route val_) {
    int sz = route_data.read_info(1);
    sz++;
    route_data.write_info(1, sz);
    route_data.write_T(sz, val_);
    return 0;
}

bool Train_System::exist_trainId(Train_Info &info_, type_trainID id_) {
    return false;
}

void Train_System::add_train(Train_Info info_) {

}

void Train_Route::write_info(char ch, vector<string> val_) {
    int sum;
    switch (ch){
        case 'p':
            //price 做前缀和 n-1项
            sum = 0;
            prices[0] = 0;
            for (int i = 0; i < val_.size(); i++){
                sum += std::stoi(val_[i]);
                prices[i + 1] = sum;
            }
            break;
        case 's':
            //stations n项
            for (int i = 0; i < val_.size(); i++)
                stations[i] = val_[i];
            break;
        case 't':
            //travelTimes 路程时间 n-1项
            for (int i = 0; i < val_.size(); i++)
                travelTimes[i] = type_time(std::stoi(val_[i]));
            break;
        case 'o':
            //stopoverTime 停靠时间 n-2项
            stopoverTimes[0] = type_time(0);
            for (int i = 0; i < val_.size(); i++)
                stopoverTimes[i + 1] = type_time(std::stoi(val_[i]));
            break;
    }
}

void Train_Route::calc_arrive() {
    arriveTimes[0] = type_time(0);
    for (int i = 1; i < num; i++)
        arriveTimes[i] = arriveTimes[i-1] + stopoverTimes[i-1] + travelTimes[i];
}
