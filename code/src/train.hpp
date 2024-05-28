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
};

struct Train_Info{
    type_trainID trainId;
    int stationNum{};
    type_Train_Route_ptr routePtr{};
    int seatNum{};
    type_time startTime; //每日发车时间
    type_time BeginDate, EndDate; //开始售卖日期
    char type{};
};

//记录途径station的trainId（关键信息,用于query_train和query_transfer）
struct Station{
    type_trainID trainId;
    int price_sum{}; //前缀和
    type_time arrive_time, stop_time;//从始发站到当前站点的时间，与停留时间
    type_time BeginDate, EndDate; //售卖日期
};

//用startDate+TrainId区分，存储Seat相关信息
struct TrainTicket{
    type_time startDate;
    type_time startTime;
    type_trainID trainId;
    int seat_res[100]{};
};

class Train_System{
private:
    LinerMemory< Train_Route, 1 > route_data;
    BPTree< Train_Info > train_data; // key: trainId value: trainInfo
    BPTree< TrainTicket > releasedTrain_data; // key: day + trainId value: ReleasedTrain
    BPTree< Station > station_data; // key: stationName + trainId    value:station

    Train_Route read_Train_Route(int pos);
    void update_Train_Route(int pos, Train_Route val_);

public:
    Train_System();

    int new_Train_Route(Train_Route val_);

    //在数据库中查询
    bool exist_trainId(Train_Info &info_, type_trainID id_);

    void add_train(Train_Info info_);

    //分别查询经过起点站和终点站的车次，得到两个vector，找出两个vector中trainId一致的车次
    //要求：1.先到起点站再到终点站
    // 2.对于每个train类型，向前推算运行时间的天数，判断是否在saleDate中
    vector<type_trainID> query_ticket();

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
