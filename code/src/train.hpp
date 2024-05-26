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
    int prices[100]; // price[i]: price between i and i+1 (前缀和优化？)
    type_time stopoverTimes[100]; // [0]空着
    type_time travelTimes[100]; // 0-based
    type_time startTime; //每日发车时间
    type_time BeginDate, EndDate; //开始售卖日期
    char Type;
};

//记录途径station的trainId（关键信息,用于query_train和query_transfer）
struct station{

};

//用startDate+TrainId区分，存储Seat相关信息
struct ReleasedTrain{

};

class Train_System{
private:

public:


    //分别查询经过起点站和终点站的车次，得到两个vector，找出两个vector中trainId一致的车次
    //要求：1.先到起点站再到终点站 2.对于每个train类型，向前推算运行时间的天数，判断是否在saleDate中
    vector<type_trainID> query_ticket();

    //分别查询经过起点站和终点站的车次，得到两个vector
    //枚举两个vector中车次
    //1.trainId不能相同
    std::pair<type_trainID, type_trainID> query_transfer();
};


#endif //TICKET_SYSTEM_2024_TRAIN_HPP
