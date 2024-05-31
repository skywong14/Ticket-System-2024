//
// Created by skywa on 2024/5/25.
//
#ifndef TICKET_SYSTEM_2024_TRAIN_HPP
#define TICKET_SYSTEM_2024_TRAIN_HPP

#include "../tools/BPTree.hpp"
#include "../tools/MyTools.hpp"
#include "../tools/LinerMemory.h"

struct Single_Pass{
    int num{}, unit_price{};
    char trainType{};
    type_time date; //列车发车的日期
    type_time setOffTime; //发车时间
    type_trainID trainId;
    type_stationName startStation, endStation;
    int startStationPos{}, endStationPos{}; //0-based
    type_time beginTime, endTime;
    string to_string();
};
using Single_Pass_Pair = std::pair<Single_Pass, Single_Pass>;

using type_Train_Route_ptr = int;
struct Train_Route{
    int num{}; //站数，stations对应0 ~ num-1
    int prices[100]{}; // price[0] = 0, 前缀和
    type_time stopoverTimes[100]; // [0]空着，每个站停靠时间
    type_time travelTimes[100]; // 路程时间， 从i_th -> i+1_th 0-based
    type_time arriveTimes[100]; // 从发车起，到达第i个站所需时间
    type_stationName stations[100]; //stations[0]:StartStation
    void write_info(char ch, vector<string> val_);
    void calc_arrive();
    int search_station(type_stationName name_);
};

using type_Seat_Info_ptr = int;
struct Seat_Info{
    int seat_num{};
    int seat_sell[100]{}; //seat_sell[0]：第0站->第1站
};

struct Train_Info{
    type_trainID trainId;
    int stationNum{};
    type_Train_Route_ptr routePtr{};
    int seatNum{};
    type_time startTime; //每日发车时间
    type_time BeginDate, EndDate; //开始售卖日期
    bool released = false;
    char type{};
    bool operator<(const Train_Info& other) const { return trainId < other.trainId; }
    bool operator==(const Train_Info& other) const { return trainId == other.trainId; }
    bool operator>(const Train_Info& other) const { return trainId > other.trainId; }
    bool operator!=(const Train_Info& other) const { return trainId != other.trainId; }
    bool operator<=(const Train_Info& other) const { return trainId <= other.trainId; }
    bool operator>=(const Train_Info& other) const { return trainId >= other.trainId; }
};

//记录途径station的trainId（关键信息,用于query_train和query_transfer）
struct Station{
    type_trainID trainId;
    int priceSum{}; //前缀和
    type_time startTime;
    int pos{}; //第pos_th个站
    type_stationName cur_station;
    type_time arriveTime, stopTime;//从始发站到当前站点的时间，与停留时间
    type_time BeginDate, EndDate; //售卖日期
    bool operator<(const Station& other) const { return trainId < other.trainId; }
    bool operator==(const Station& other) const { return trainId == other.trainId; }
    bool operator>(const Station& other) const { return trainId > other.trainId; }
    bool operator!=(const Station& other) const { return trainId != other.trainId; }
    bool operator<=(const Station& other) const { return trainId <= other.trainId; }
    bool operator>=(const Station& other) const { return trainId >= other.trainId; }
};

//用startDate+TrainId区分，存储Seat相关信息
struct DayTicket{
    type_time date;
    type_trainID trainId;
    type_Seat_Info_ptr seatInfo_ptr{};
    bool operator<(const DayTicket& other) const { return trainId < other.trainId; }
    bool operator==(const DayTicket& other) const { return trainId == other.trainId; }
    bool operator>(const DayTicket& other) const { return trainId > other.trainId; }
    bool operator!=(const DayTicket& other) const { return trainId != other.trainId; }
    bool operator<=(const DayTicket& other) const { return trainId <= other.trainId; }
    bool operator>=(const DayTicket& other) const { return trainId >= other.trainId; }
};


Single_Pass get_Single_Pass(type_time cur_date, Train_Info trainInfo, Train_Route route, int num, type_stationName staStart, type_stationName staEnd);


//for query_ticket
struct CmpSinglePass_Time {
    bool operator()(const Single_Pass& sp1, const Single_Pass& sp2) const {
        if ((sp1.endTime - sp1.beginTime) != (sp2.endTime - sp2.beginTime))
            return (sp1.endTime - sp1.beginTime) < (sp2.endTime - sp2.beginTime);
        return sp1.trainId < sp2.trainId;
    }
};
struct CmpSinglePass_Cost {
    bool operator()(const Single_Pass& sp1, const Single_Pass& sp2) const {
        if (sp1.unit_price != sp2.unit_price)
            return sp1.unit_price < sp2.unit_price;
        return sp1.trainId < sp2.trainId;
    }
};
template<typename Compare>
vector<Single_Pass> merge_Single_Pass(vector<Single_Pass> left, vector<Single_Pass> right, Compare cmp) {
    vector<Single_Pass> result;
    auto left_it = left.begin();
    auto right_it = right.begin();
    while (left_it != left.end() && right_it != right.end()) {
        if (cmp(*left_it, *right_it)) {
            result.push_back(*left_it); ++left_it;
        } else {
            result.push_back(*right_it); ++right_it;
        }
    }
    while (left_it != left.end()) { result.push_back(*left_it); ++left_it; }
    while (right_it != right.end()) { result.push_back(*right_it); ++right_it; }
    return result;
}
template<typename Compare>
vector<Single_Pass> sort_Single_Pass(vector<Single_Pass> vec, Compare cmp) {
    if (vec.size() <= 1) return vec;
    auto middle = vec.begin() + (vec.size() / 2);
    vector<Single_Pass> left;
    for (auto it = vec.begin(); it != middle; it++) left.push_back(*it);
    vector<Single_Pass> right;
    for (auto it = middle; it != vec.end(); it++) right.push_back(*it);
    left = sort_Single_Pass(left, cmp);
    right = sort_Single_Pass(right, cmp);
    return merge_Single_Pass(left, right, cmp);
}

//for query_transfer
struct CmpSinglePass_Time_First {
    bool operator()(const Single_Pass& sp1, const Single_Pass& sp2) const {
        if ((sp1.date + sp1.endTime + sp1.setOffTime) != (sp2.date + sp2.endTime + sp2.setOffTime))
            return (sp1.date + sp1.endTime + sp1.setOffTime) < (sp2.date + sp2.endTime + sp2.setOffTime);

        return sp1.unit_price < sp2.unit_price;
    }
};
struct CmpSinglePass_Cost_First {
    bool operator()(const Single_Pass& sp1, const Single_Pass& sp2) const {
        if (sp1.unit_price != sp2.unit_price)
            return sp1.unit_price < sp2.unit_price;
        return (sp1.date + sp1.endTime + sp1.setOffTime) < (sp2.date + sp2.endTime + sp2.setOffTime);
    }
};
struct CmpSinglePassPair_Time {
    bool operator()(const Single_Pass_Pair& sp1, const Single_Pass_Pair& sp2) const {
        // 总时间作为第一关键字，总价格作为第二关键字，第一辆车的 Train ID 作为第三关键字，第二辆车 Train ID 作为第四关键字
        // 计算两个票对的总旅行时间
        int total_time_sp1 = ((sp1.second.date + sp1.second.endTime + sp1.second.setOffTime) -
                             (sp1.first.date + sp1.first.beginTime + sp1.first.setOffTime)).standard;
        int total_time_sp2 = ((sp2.second.date + sp2.second.endTime + sp2.second.setOffTime) -
                              (sp2.first.date + sp2.first.beginTime + sp2.first.setOffTime)).standard;
        if (total_time_sp1 != total_time_sp2)  return total_time_sp1 < total_time_sp2;
        // 计算两个票对的总价格
        int total_price_sp1 = sp1.first.unit_price + sp1.second.unit_price;
        int total_price_sp2 = sp2.first.unit_price + sp2.second.unit_price;
        if (total_price_sp1 != total_price_sp2) return total_price_sp1 < total_price_sp2;
        // 比较第一辆列车的ID
        if (sp1.first.trainId != sp2.first.trainId)  return sp1.first.trainId < sp2.first.trainId;
        // 比较第二辆列车的ID
        return sp1.second.trainId < sp2.second.trainId;
    }
};
struct CmpSinglePassPair_Cost {
    bool operator()(const Single_Pass_Pair& sp1, const Single_Pass_Pair& sp2) const {
        // 总价格作为第一关键字，总时间作为第二关键字，第一辆车的 Train ID 作为第三关键字，第二辆车 Train ID 作为第四关键字。
        // 计算两个票对的总价格
        int total_price_sp1 = sp1.first.unit_price + sp1.second.unit_price;
        int total_price_sp2 = sp2.first.unit_price + sp2.second.unit_price;
        if (total_price_sp1 != total_price_sp2) return total_price_sp1 < total_price_sp2;
        // 计算两个票对的总旅行时间
        int total_time_sp1 = ((sp1.second.date + sp1.second.endTime + sp1.second.setOffTime) -
                              (sp1.first.date + sp1.first.beginTime + sp1.first.setOffTime)).standard;
        int total_time_sp2 = ((sp2.second.date + sp2.second.endTime + sp2.second.setOffTime) -
                              (sp2.first.date + sp2.first.beginTime + sp2.first.setOffTime)).standard;
        if (total_time_sp1 != total_time_sp2)  return total_time_sp1 < total_time_sp2;
        // 比较第一辆列车的ID
        if (sp1.first.trainId != sp2.first.trainId)  return sp1.first.trainId < sp2.first.trainId;
        // 比较第二辆列车的ID
        return sp1.second.trainId < sp2.second.trainId;
    }
};
template<typename Compare>
vector<Single_Pass_Pair> merge_Single_Pass_Pair(vector<Single_Pass_Pair> left, vector<Single_Pass_Pair> right, Compare cmp) {
    vector<Single_Pass_Pair> result;
    auto left_it = left.begin();
    auto right_it = right.begin();
    while (left_it != left.end() && right_it != right.end()) {
        if (cmp(*left_it, *right_it)) {
            result.push_back(*left_it); ++left_it;
        } else {
            result.push_back(*right_it); ++right_it;
        }
    }
    while (left_it != left.end()) { result.push_back(*left_it); ++left_it; }
    while (right_it != right.end()) { result.push_back(*right_it); ++right_it; }
    return result;
}
template<typename Compare>
vector<Single_Pass_Pair> sort_Single_Pass_Pair(vector<Single_Pass_Pair> vec, Compare cmp) {
    if (vec.size() <= 1) return vec;
    auto middle = vec.begin() + (vec.size() / 2);
    vector<Single_Pass_Pair> left;
    for (auto it = vec.begin(); it != middle; it++) left.push_back(*it);
    vector<Single_Pass_Pair> right;
    for (auto it = middle; it != vec.end(); it++) right.push_back(*it);
    left = sort_Single_Pass(left, cmp);
    right = sort_Single_Pass(right, cmp);
    return merge_Single_Pass(left, right, cmp);
}


class Train_System{
private:
    LinerMemory< Seat_Info, 1 > seat_data;
    LinerMemory< Train_Route, 1 > route_data;
    BPTree< Train_Info, 3500, 80, 300> train_data; // key: trainId value: trainInfo
    BPTree< DayTicket > ticket_data; // key: day + '|' + trainId value: DayTicket
    BPTree< Station > station_data; // key: stationName   value: Station
public:
    Train_System();

    Train_Route read_Train_Route(int pos);
    void update_Train_Route(int pos, Train_Route val_);
    Seat_Info read_Seat_Info(int pos);
    void update_Seat_Info(int pos, Seat_Info val_);

    int new_Train_Route(Train_Route val_);
    int new_Seat_Info(Seat_Info val_);

    Seat_Info query_seat_info(type_time date, type_trainID trainId);

    bool exist_trainId(Train_Info &info_, type_trainID id_);

    bool exist_DayTicket(DayTicket &day_ticket, type_time date, type_trainID trainId);

    void add_train(type_trainID id_, Train_Info info_);

    void delete_train(type_trainID trainId, Train_Info info_);

    void modify_train(type_trainID trainId, Train_Info pre_info_, Train_Info new_info_);

    void release_train(type_trainID trainId, Train_Info info_);

    void add_stationTrain(type_stationName stationName, Station info_);

    ReturnMode query_train(type_time date_, type_trainID trainId);

    int maximum_seats(type_time date, type_trainID trainId, int pos1, int pos2);
    int maximum_seats(Train_Route route_, DayTicket day_ticket_, type_stationName sta1, type_stationName sta2);

    type_Seat_Info_ptr refund_ticket(Single_Pass singlePass);
    void buy_ticket(DayTicket dayTicket, int posStart, int posEnd, int num);

    vector<Station> query_related_train(type_stationName stationName);

    //分别查询经过起点站和终点站的车次，得到两个vector，找出两个vector中trainId一致的车次
    //要求：1.先到起点站再到终点站
    // 2.对于每个train类型，向前推算运行时间的天数，判断是否在saleDate中
    vector< Single_Pass > pass_by_trains(type_time cur_time, type_stationName sta1, type_stationName sta2);

    //分别查询经过起点站和终点站的车次，得到两个vector
    //枚举两个vector中车次，记为Train1, Train2
    //1.trainId不能相同，对于Train1，要求对应发车时间在售卖时间内
    //2.Train1到达中转站后 Train2存在一天有卖票
    //3.枚举中转站(a0->?->b0)：
    // Train1:途径 a0, a1, a2, a3, ...
    // Train2:途径 ...b3, b2, b1, b0
    Single_Pass the_best_train(type_trainID another_train, type_time earliest_time, type_stationName sta1, type_stationName sta2, bool time_first);

    void query_transfer(type_time leave_date, type_stationName sta1, type_stationName sta2, bool time_first);
};


#endif //TICKET_SYSTEM_2024_TRAIN_HPP
