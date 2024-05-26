//
// Created by skywa on 2024/5/26.
//
#ifndef TICKET_SYSTEM_2024_ORDER_HPP
#define TICKET_SYSTEM_2024_ORDER_HPP
#include "../tools/BPTree.hpp"
#include "../tools/MyTools.hpp"

struct Order{
    int time_stamp = 0; //时间戳，下单时间
    int num = 0, unit_price = 0;
    type_userid userid; //购票者
    type_trainID trainId;
    type_stationName startStation, endStation;
    int startStationId, endStationId; //0-based
    type_time data; //列车发车的日期
    //other...
};

//补票中
struct WaitingOrder{
    int time_stamp = 0; //时间戳，提交候补时间
    int num = 0, unit_price = 0;
    type_userid userid; //购票者
    type_trainID trainId;
    int startStationId, endStationId; //0-based
    type_time data; //列车发车的日期
};

class Order_System{
private:
    BPTree< > order_data;  //key: oderId          value: Order(struct)
    BPTree< > user_order;  //key: userId          value: orderId
    BPTree< > order_queue; //key: userId + trainId  value: WaitingOrder
public:

};

#endif //TICKET_SYSTEM_2024_ORDER_HPP
