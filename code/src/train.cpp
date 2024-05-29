//
// Created by skywa on 2024/5/28.
//
#include "train.hpp"

Train_System::Train_System() {
    seat_data.initialise("seat_data");
    route_data.initialise("route_data");
    train_data.initialise("train_data");
    ticket_data.initialise("ticket_data");
    station_data.initialise("station_data");
}

Train_Route Train_System::read_Train_Route(int pos) {
    return route_data.read_T(pos);
}
void Train_System::update_Train_Route(int pos, Train_Route val_) {
    route_data.write_T(pos, val_);
}
int Train_System::new_Train_Route(Train_Route val_) {
    int sz = route_data.read_info(1); sz++;
    route_data.write_info(1, sz);
    route_data.write_T(sz, val_);
    return sz;
}

Seat_Info Train_System::read_Seat_Info(int pos) {
    return seat_data.read_T(pos);
}

void Train_System::update_Seat_Info(int pos, Seat_Info val_) {
    seat_data.write_T(pos, val_);
}

int Train_System::new_Seat_Info(Seat_Info val_) {
    int sz = seat_data.read_info(1); sz++;
    seat_data.write_info(1, sz);
    seat_data.write_T(sz, val_);
    return sz;
}


bool Train_System::exist_trainId(Train_Info &info_, type_trainID id_) {
    vector<Train_Info> vec = train_data.search_values(id_.to_string());
    if (!vec.empty()){
        assert(vec.size() == 1);
        info_ = vec[0];
        return true;
    }
    return false;
}

void Train_System::add_train(type_trainID id_, Train_Info info_) {
    train_data.insert(id_.to_string(), info_);
}

void Train_System::delete_train(type_trainID trainId, Train_Info info_) {
    train_data.erase(trainId.to_string(), info_);
}


void Train_System::modify_train(type_trainID trainId, Train_Info pre_info_, Train_Info new_info_) {
    delete_train(trainId, pre_info_);
    add_train(trainId, new_info_);
}

void Train_System::release_train(type_trainID trainId, Train_Info info_) {
    //更新released
    Train_Info new_info_ = info_;
    Station station_info;
    type_stationName cur_station;
    new_info_.released = true;
    modify_train(trainId, info_, new_info_);

    //将途径的每个station的信息写入
    Train_Route cur_route = read_Train_Route(info_.routePtr);

    station_info.trainId = trainId;
    station_info.startTime = info_.startTime;

    cur_route.arriveTimes[0] = NegMaxTime;
    cur_route.stopoverTimes[0] = MaxTime;
    cur_route.stopoverTimes[cur_route.num - 1] = MaxTime;

    for (int i = 0; i < cur_route.num; i++){
        cur_station = cur_route.stations[i];
        station_info.BeginDate = info_.BeginDate;
        station_info.EndDate = info_.EndDate;
        station_info.arriveTime = cur_route.arriveTimes[i];
        station_info.stopTime = cur_route.stopoverTimes[i];
        station_info.priceSum = cur_route.prices[i];
        add_stationTrain(cur_station, station_info);
    }

    //创建（天数）趟车次
    int days = (info_.EndDate - info_.BeginDate).Days() + 1;

    DayTicket dayTicket;
    dayTicket.trainId = trainId;

    Seat_Info seat_info;
    seat_info.seat_num = info_.seatNum;

    type_time cur_time = info_.BeginDate;

    for (int i = 0; i < days; i++){
        dayTicket.seatInfo_ptr = new_Seat_Info(seat_info);
        dayTicket.date = cur_time;
        ticket_data.insert( to_index(cur_time, trainId) , dayTicket);
        cur_time = cur_time + A_Day;
    }
}

void Train_System::add_stationTrain(type_stationName stationName, Station info_) {
    station_data.insert(stationName.to_string(), info_);
}

vector<Station> Train_System::query_related_train(type_stationName stationName) {
    return station_data.search_values(stationName.to_string());
}

ReturnMode Train_System::query_ticket(type_time cur_time, type_stationName sta1, type_stationName sta2, const string &type_) {
    vector<Station> trains1 = query_related_train(sta1);
    vector<Station> trains2 = query_related_train(sta2);
    vector<Station> trains3 = shared_elements( trains1, trains2 );
    trains1.clear(); trains2.clear();
    //trains3 为可能的车
    //todo

    return ReturnMode::Correct;
}

ReturnMode Train_System::query_train(type_time date_, type_trainID trainId) {
    Train_Info train_info;
    if (!exist_trainId(train_info, trainId)) return ReturnMode::Invalid_Operation;
    if (date_ < train_info.startTime || date_ > train_info.EndDate) return ReturnMode::Wrong_Value;
    Train_Route route = read_Train_Route(train_info.routePtr);
    type_time base_time = train_info.startTime + train_info.BeginDate;

    if (!train_info.released){
        //未发布
        std::cout<<trainId<<' '<<train_info.type<<std::endl;
        //起点
        std::cout << route.stations[0] << ' ' << empty_time_string()
                  << " -> " << base_time.to_string()
                  << route.prices[0] << ' ' << train_info.seatNum << std::endl;
        //中间
        for (int i = 1; i < route.num - 1; i++){
            std::cout << route.stations[i] << ' ' << (base_time + route.arriveTimes[i]).to_string()
                      << " -> " << (base_time + route.arriveTimes[i] + route.stopoverTimes[i]).to_string()
                      << route.prices[i] << ' ' << train_info.seatNum << std::endl;
        }
        //终点
        std::cout << route.stations[route.num - 1] << ' ' << (base_time + route.arriveTimes[route.num - 1]).to_string()
                  << " -> " << empty_time_string()
                  << route.prices[route.num - 1] << ' ' << 'x' << std::endl;
    } else {
        //已发布
        std::cout<<trainId<<' '<<train_info.type<<std::endl;

        vector<DayTicket> day_ticket = ticket_data.search_values(to_index(date_, trainId));
        assert(day_ticket.size() == 1); // for debug
        assert(day_ticket[0].seatInfo_ptr > 0); // for debug

        Seat_Info seat_info = read_Seat_Info(day_ticket[0].seatInfo_ptr);

        //起点
        std::cout << route.stations[0] << ' ' << empty_time_string()
                  << " -> " << base_time.to_string()
                  << route.prices[0] << ' ' << train_info.seatNum << std::endl;
        //中间
        for (int i = 1; i < route.num - 1; i++){
            std::cout << route.stations[i] << ' ' << (base_time + route.arriveTimes[i]).to_string()
                      << " -> " << (base_time + route.arriveTimes[i] + route.stopoverTimes[i]).to_string()
                      << route.prices[i] << ' ' << seat_info.seat_num - seat_info.seat_sell[i - 1] << std::endl;
        }
        //终点
        std::cout << route.stations[route.num - 1] << ' ' << (base_time + route.arriveTimes[route.num - 1]).to_string()
                  << " -> " << empty_time_string()
                  << route.prices[route.num - 1] << ' ' << 'x' << std::endl;
    }

    return ReturnMode::Correct;
}

bool Train_System::exist_DayTicket(DayTicket &day_ticket, type_time date, type_trainID trainId) {
    vector< DayTicket> val = ticket_data.search_values(to_index(date, trainId));
    assert(val.size() <= 1);
    if (!val.empty()){
        day_ticket = val[0];
        return true;
    }
    return false;
}

int
Train_System::maximum_seats(Train_Route route_, DayTicket day_ticket_, type_stationName sta1, type_stationName sta2) {
    int pos1 = route_.search_station(sta1), pos2 = route_.search_station(sta2);
    Seat_Info seatInfo = read_Seat_Info(day_ticket_.seatInfo_ptr);
    int num = 0;
    for (int i = pos1; i < pos2; i++)
        num = std::max(num, seatInfo.seat_sell[i]);
    return seatInfo.seat_num - num;
}

void Train_System::buy_ticket(DayTicket dayTicket, int posStart, int posEnd, int num) {
    type_time cur_day = dayTicket.date;
    type_trainID cur_train = dayTicket.trainId;

    Seat_Info seatInfo = seat_data.read_T(dayTicket.seatInfo_ptr);
    for (int i = posStart; i < posEnd; i++){
        seatInfo.seat_sell[i] += num;
        assert(seatInfo.seat_sell[i] <= seatInfo.seat_num);
    }
    seat_data.write_T(dayTicket.seatInfo_ptr, seatInfo);
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

int Train_Route::search_station(type_stationName name_) {
    for (int i = 0; i < num; i++)
        if (name_ == stations[i]) return i;
    return -1; //not found
}
