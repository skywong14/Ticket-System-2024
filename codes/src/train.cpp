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
        station_info.pos = i;
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

vector< Single_Pass > Train_System::pass_by_trains(type_time leave_date, type_stationName sta1, type_stationName sta2) {
    vector<Station> trains1 = query_related_train(sta1);
    vector<Station> trains2 = query_related_train(sta2);
    //trains1 & trains2 is ordered

    Single_Pass singlePass;
    vector<Single_Pass> ret;

    Station station1, station2;

    type_time set_off_date;

    //要求：1.出发/到达顺序正确  2.日期合法
    auto it1 = trains1.begin(), it2 = trains2.begin();
    while (it1 != trains1.end() && it2 != trains2.end()){
        station1 = *it1; station2 = *it2;
        if (station1 == station2){
            if (station1.pos < station2.pos){

                set_off_date = setOffDate( leave_date, station1.startTime + station1.arriveTime + station1.stopTime );

                if ( set_off_date >= station1.BeginDate && set_off_date <= station1.EndDate ){
                    singlePass.date = set_off_date;
                    singlePass.setOffTime = station1.startTime;
                    singlePass.trainId = station1.trainId;
                    singlePass.unit_price = station2.priceSum - station1.priceSum;
                    singlePass.beginTime = station1.arriveTime + station1.stopTime;
                    singlePass.endTime = station2.arriveTime;
                    singlePass.startStation = sta1;
                    singlePass.endStation = sta2;
                    singlePass.startStationPos = station1.pos;
                    singlePass.endStationPos = station2.pos;

                    ret.push_back(singlePass);
                }
            }

            it1++; it2++;
        } else {
            if (station1 < station2)it1++;
            else it2++;
        }

    }
    return ret;
}

Single_Pass Train_System::the_best_train(type_trainID another_train, type_time earliest_time,
                                         type_stationName sta1, type_stationName sta2, bool time_first) {
    //不同于another_train，离开时间大于等于earliest_time，该站为sta1，目的地为sta2
    //time_first表示time优先或cost优先
    CmpSinglePass_Time_First cmpSinglePass_TimeFirst;
    CmpSinglePass_Cost_First cmpSinglePass_CostFirst;
    vector<Station> trains1 = query_related_train(sta1);
    vector<Station> trains2 = query_related_train(sta2);
    Single_Pass best_train;  best_train.trainId = type_trainID("");

    //要求：1.出发/到达顺序正确  2.时间合法
    bool first_train = true;
    Single_Pass singlePass;
    Station station1, station2;
    type_time set_off_date;
    auto it1 = trains1.begin(), it2 = trains2.begin();
    while (it1 != trains1.end() && it2 != trains2.end()){
        station1 = *it1; station2 = *it2;
        if (station1 == station2){
            if (station1.pos < station2.pos && station1.trainId != another_train){
                //set_off_date + beginTime + arriveTime + stopTime >= earliest_time
                //set_off_date >= date(earliest_time - beginTime - arriveTime - stopTime)
                set_off_date = (earliest_time - station1.startTime - station1.arriveTime - station1.stopTime).ceiling_days();


                if ( set_off_date <= station1.EndDate ){
                    if (set_off_date < station1.BeginDate)
                        singlePass.date = station1.BeginDate;
                    else
                        singlePass.date = set_off_date;
                    singlePass.setOffTime = station1.startTime;
                    singlePass.trainId = station1.trainId;
                    singlePass.unit_price = station2.priceSum - station1.priceSum;
                    singlePass.beginTime = station1.arriveTime + station1.stopTime;
                    singlePass.endTime = station2.arriveTime;
                    singlePass.startStation = sta1;
                    singlePass.endStation = sta2;
                    singlePass.startStationPos = station1.pos;
                    singlePass.endStationPos = station2.pos;

                    if (first_train){
                        best_train = singlePass;
                        first_train = false;
                    } else {
                        if (time_first){
                            if (cmpSinglePass_TimeFirst(singlePass, best_train))
                                best_train = singlePass;
                        } else {
                            if (cmpSinglePass_CostFirst(singlePass, best_train))
                                best_train = singlePass;
                        }
                    }
                }
            }
            it1++; it2++;
        } else {
            if (station1 < station2) it1++;
            else it2++;
        }
    }
    return best_train;
}

void Train_System::query_transfer(type_time leave_date, type_stationName sta1, type_stationName sta2, bool time_first) {
    vector<Station> trains1 = query_related_train(sta1);
    vector<Station> trains2 = query_related_train(sta2);
    if (trains1.empty() || trains2.empty()) {
        std::cout<<0<<std::endl;
        return;
    }
    CmpSinglePassPair_Time cmpSinglePassPair_Time;
    CmpSinglePassPair_Cost cmpSinglePassPair_Cost;
    Single_Pass_Pair best_pair, ret_pair; bool first_pair = true;
    Station station1, station2;
    Train_Info cur_train;
    Train_Route route;
    int price0;
    Single_Pass pass1, pass2;
    for (int i = 0; i < trains1.size(); i++){
        station1 = trains1[i];
        exist_trainId(cur_train, station1.trainId);
        route = read_Train_Route(cur_train.routePtr);

        price0 = station1.priceSum; //当前站
        assert(price0 == route.prices[station1.pos]);

        pass1.startStation = sta1;
        pass1.startStationPos = station1.pos;
        pass1.trainId = station1.trainId;
        pass1.date = setOffDate( leave_date, station1.startTime + station1.arriveTime + station1.stopTime);
        pass1.setOffTime = station1.startTime;
        pass1.beginTime = station1.arriveTime + station1.stopTime;
        pass1.trainType = cur_train.type;

        if (pass1.date < station1.BeginDate || pass1.date > station1.EndDate) continue;

        for (int j = station1.pos + 1; j < route.num; j++){
            pass1.unit_price = route.prices[j] - price0;
            pass1.endStationPos = j;
            pass1.endStation = route.stations[j];
            pass1.endTime = route.arriveTimes[j];

            pass2 = the_best_train( pass1.trainId, pass1.date + pass1.setOffTime + pass1.endTime,
                                    pass1.endStation, sta2, time_first);

            if (pass2.trainId != type_trainID("")){
                //exist
                ret_pair = Single_Pass_Pair(pass1, pass2);
                if (first_pair){
                    best_pair = ret_pair;
                    first_pair = false;
                } else {
                    if (time_first){
                        if (cmpSinglePassPair_Time(ret_pair, best_pair))
                            best_pair = ret_pair;
                    } else {
                        if (cmpSinglePassPair_Cost(ret_pair, best_pair))
                            best_pair = ret_pair;
                    }
                }
            }
        }
    }
    if (first_pair) std::cout<<0<<std::endl;
    else {
        pass1 = best_pair.first;
        pass2 = best_pair.second;
        int tmp_num = maximum_seats(pass1.date, pass1.trainId,pass1.startStationPos, pass1.endStationPos);
        std::cout<<pass1.to_string()<<' '<< tmp_num <<std::endl;
        tmp_num = maximum_seats(pass2.date, pass2.trainId, pass2.startStationPos, pass2.endStationPos);
        std::cout<<pass2.to_string()<<' '<< tmp_num <<std::endl;
    }
}

ReturnMode Train_System::query_train(type_time date_, type_trainID trainId) {
    Train_Info train_info;
    if (!exist_trainId(train_info, trainId)) return ReturnMode::Invalid_Operation;
    if (date_ < train_info.BeginDate || date_ > train_info.EndDate) return ReturnMode::Wrong_Value;
    Train_Route route = read_Train_Route(train_info.routePtr);
    type_time base_time = train_info.startTime + date_;

    if (!train_info.released){
        //未发布
        std::cout<<trainId<<' '<<train_info.type<<std::endl;
        //起点
        std::cout << route.stations[0] << ' ' << empty_time_string()
                  << " -> " << base_time.to_string() <<' '
                  << route.prices[0] << ' ' << train_info.seatNum << std::endl;
        //中间
        for (int i = 1; i < route.num - 1; i++){
            std::cout << route.stations[i] << ' ' << (base_time + route.arriveTimes[i]).to_string()
                      << " -> " << (base_time + route.arriveTimes[i] + route.stopoverTimes[i]).to_string() <<' '
                      << route.prices[i] << ' ' << train_info.seatNum << std::endl;
        }
        //终点
        std::cout << route.stations[route.num - 1] << ' ' << (base_time + route.arriveTimes[route.num - 1]).to_string()
                  << " -> " << empty_time_string() <<' '
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
                  << " -> " << base_time.to_string() <<' '
                  << route.prices[0] << ' ' << train_info.seatNum - seat_info.seat_sell[0]  << std::endl;
        //中间
        for (int i = 1; i < route.num - 1; i++){
            std::cout << route.stations[i] << ' ' << (base_time + route.arriveTimes[i]).to_string()
                      << " -> " << (base_time + route.arriveTimes[i] + route.stopoverTimes[i]).to_string() <<' '
                      << route.prices[i] << ' ' << seat_info.seat_num - seat_info.seat_sell[i] << std::endl;
        }
        //终点
        std::cout << route.stations[route.num - 1] << ' ' << (base_time + route.arriveTimes[route.num - 1]).to_string()
                  << " -> " << empty_time_string() <<' '
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

int Train_System::maximum_seats(type_time date, type_trainID trainId, int pos1, int pos2) {
    Seat_Info seatInfo = query_seat_info(date, trainId);
    int num = 0;
    for (int i = pos1; i < pos2; i++)
        num = std::max(num, seatInfo.seat_sell[i]);
    return seatInfo.seat_num - num;
}
int Train_System::maximum_seats(Train_Route route_, DayTicket day_ticket_, type_stationName sta1, type_stationName sta2) {
    int pos1 = route_.search_station(sta1), pos2 = route_.search_station(sta2);
    Seat_Info seatInfo = read_Seat_Info(day_ticket_.seatInfo_ptr);
    int num = 0;
    for (int i = pos1; i < pos2; i++)
        num = std::max(num, seatInfo.seat_sell[i]);
    return seatInfo.seat_num - num;
}
//todo long long

void Train_System::buy_ticket(DayTicket dayTicket, int posStart, int posEnd, int num) {
    assert(dayTicket.seatInfo_ptr > 0);
    Seat_Info seatInfo = seat_data.read_T(dayTicket.seatInfo_ptr);
    for (int i = posStart; i < posEnd; i++){
        seatInfo.seat_sell[i] += num;
        assert(seatInfo.seat_sell[i] <= seatInfo.seat_num);
    }
    seat_data.write_T(dayTicket.seatInfo_ptr, seatInfo);
}

type_Seat_Info_ptr Train_System::refund_ticket(Single_Pass singlePass) {
    DayTicket dayTicket = ticket_data.search_values(to_index(singlePass.date, singlePass.trainId))[0];
    buy_ticket(dayTicket,singlePass.startStationPos, singlePass.endStationPos, -singlePass.num );
    return  dayTicket.seatInfo_ptr;
}

Seat_Info Train_System::query_seat_info(type_time date, type_trainID trainId) {
    vector<DayTicket> ret = ticket_data.search_values(to_index(date, trainId));
    assert(ret.size() == 1);
    return read_Seat_Info(ret[0].seatInfo_ptr);
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
        arriveTimes[i] = arriveTimes[i-1] + stopoverTimes[i-1] + travelTimes[i - 1];
}

int Train_Route::search_station(type_stationName name_) {
    for (int i = 0; i < num; i++)
        if (name_ == stations[i]) return i;
    return -1; //not found
}


Single_Pass get_Single_Pass(type_time cur_date, Train_Info cur_train_info, Train_Route cur_route, int num, type_stationName staStart, type_stationName staEnd){
    Single_Pass val;
    val.trainId = cur_train_info.trainId;
    val.trainType = cur_train_info.type;
    val.num = num;
    val.setOffTime = cur_train_info.startTime;
    val.startStationPos =  cur_route.search_station(staStart);
    val.endStationPos = cur_route.search_station(staEnd);
    val.startStation = cur_route.stations[val.startStationPos];
    val.endStation = cur_route.stations[val.endStationPos];

    val.date = setOffDate(cur_date, cur_train_info.startTime, cur_route.stopoverTimes[val.startStationPos], cur_route.arriveTimes[val.startStationPos]); //发车日
    val.beginTime = type_time(cur_route.arriveTimes[val.startStationPos] + cur_route.stopoverTimes[val.startStationPos]);
    val.endTime = type_time(cur_route.arriveTimes[val.endStationPos]);
    val.unit_price = cur_route.prices[val.endStationPos] - cur_route.prices[val.startStationPos];
    return val;
}

