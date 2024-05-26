//
// Created by skywa on 2024/5/25.
//
#ifndef TICKET_SYSTEM_2024_TRAIN_HPP
#define TICKET_SYSTEM_2024_TRAIN_HPP

#include "../tools/BPTree.hpp"
#include "../tools/MyTools.hpp"

struct Train_Info{
    type_trainID trainId;
    int stationNum;
    type_stationName stations[100]; //stations[0]:StartStation
    int seatNum;
    int prices[100]; // price[0] = 0, 前缀和
    type_time stopoverTimes[100]; // [0]空着，每个站停靠时间
    type_time arriveTimes[100]; // 从发车起，到达第i个站所需时间
    type_time startTime; //每日发车时间
    type_time BeginDate, EndDate; //开始售卖日期
    char Type;
};

//记录途径station的trainId（关键信息,用于query_train和query_transfer）
struct station{
    type_trainID trainId;
    int price_sum; //前缀和
    type_time arrive_time, stoptime;//从始发站到当前站点的时间，与停留时间
    type_time BeginDate, EndDate; //售卖日期
};

//用startDate+TrainId区分，存储Seat相关信息
struct ReleasedTrain{
    type_time startDate;
    type_time startTime;
    type_trainID trainId;
    int seat_res[100];
};

class Train_System{
private:
    BPTree< > train_data; // key: trainId value: trainInfo
    BPTree< > releasedTrain_data; // key: day + trainId value: ReleasedTrain
    BPTree< > station_data; // key: stationName + trainId    value:station
public:


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
