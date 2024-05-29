//
// Created by skywa on 2024/5/25.
//
#ifndef TICKET_SYSTEM_2024_TRAIN_HPP
#define TICKET_SYSTEM_2024_TRAIN_HPP

#include "../tools/BPTree.hpp"
#include "../tools/MyTools.hpp"
#include "../tools/LinerMemory.h"

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


class Train_System{
private:
    LinerMemory< Seat_Info, 1 > seat_data;
    LinerMemory< Train_Route, 1 > route_data;
    BPTree< Train_Info > train_data; // key: trainId value: trainInfo
    BPTree< DayTicket > ticket_data; // key: day + '|' + trainId value: DayTicket
    BPTree< Station > station_data; // key: stationName   value:trainId
public:
    Train_System();

    Train_Route read_Train_Route(int pos);
    void update_Train_Route(int pos, Train_Route val_);
    Seat_Info read_Seat_Info(int pos);
    void update_Seat_Info(int pos, Seat_Info val_);

    int new_Train_Route(Train_Route val_);
    int new_Seat_Info(Seat_Info val_);

    bool exist_trainId(Train_Info &info_, type_trainID id_);

    bool exist_DayTicket(DayTicket &day_ticket, type_time date, type_trainID trainId);

    void add_train(type_trainID id_, Train_Info info_);

    void delete_train(type_trainID trainId, Train_Info info_);

    void modify_train(type_trainID trainId, Train_Info pre_info_, Train_Info new_info_);

    void release_train(type_trainID trainId, Train_Info info_);

    void add_stationTrain(type_stationName stationName, Station info_);

    ReturnMode query_train(type_time date_, type_trainID trainId);

    int maximum_seats(Train_Route route_, DayTicket day_ticket_, type_stationName sta1, type_stationName sta2);

    void buy_ticket(DayTicket dayTicket, int posStart, int posEnd, int num);

    vector<Station> query_related_train(type_stationName stationName);

    //分别查询经过起点站和终点站的车次，得到两个vector，找出两个vector中trainId一致的车次
    //要求：1.先到起点站再到终点站
    // 2.对于每个train类型，向前推算运行时间的天数，判断是否在saleDate中
    ReturnMode query_ticket(type_time cur_time, type_stationName sta1, type_stationName sta2, const string& type_);

    //分别查询经过起点站和终点站的车次，得到两个vector
    //枚举两个vector中车次，记为Train1, Train2
    //1.trainId不能相同，对于Train1，要求对应发车时间在售卖时间内
    //2.Train1到达中转站后 Train2存在一天有卖票
    //3.枚举中转站(a0->?->b0)：
    // Train1:途径 a0, a1, a2, a3, ...
    // Train2:途径 ...b3, b2, b1, b0
    std::pair<type_trainID, type_trainID> query_transfer();


};


#endif //TICKET_SYSTEM_2024_TRAIN_HPP
