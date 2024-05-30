//
// Created by skywa on 2024/5/26.
//
#ifndef TICKET_SYSTEM_2024_ORDER_HPP
#define TICKET_SYSTEM_2024_ORDER_HPP
#include "../tools/BPTree.hpp"
#include "../tools/MyTools.hpp"
#include "train.hpp"


struct Order{
    int orderId{}; //订单编号
    type_userid userid; //购票者
    Single_Pass singlePass;

    bool operator<(const Order& other) const { return orderId < other.orderId; }
    bool operator==(const Order& other) const { return orderId == other.orderId; }
    bool operator>(const Order& other) const { return orderId > other.orderId; }
    bool operator!=(const Order& other) const { return orderId != other.orderId; }
    bool operator<=(const Order& other) const { return orderId <= other.orderId; }
    bool operator>=(const Order& other) const { return orderId >= other.orderId; }
    string to_string(int state);
};

struct OrderId{
    int state{}; //0:pending 1:success 2:refund
    int orderId{};
    bool operator<(const OrderId& other) const { return orderId < other.orderId; }
    bool operator==(const OrderId& other) const { return orderId == other.orderId; }
    bool operator>(const OrderId& other) const { return orderId > other.orderId; }
    bool operator!=(const OrderId& other) const { return orderId != other.orderId; }
    bool operator<=(const OrderId& other) const { return orderId <= other.orderId; }
    bool operator>=(const OrderId& other) const { return orderId >= other.orderId; }
};

class Order_System{
private:
    BPTree< Order > order_data; //key: oderId   value: Order
    BPTree< Order > waitingOrder_data;  //key: oderId   value: Order
    BPTree< OrderId > userOrder_data;  //key: userId  value: orderId
    BPTree< int > waitingQueue_data; //key: day + '|' + trainId  value: int(orderId)
public:
    Order_System();
    int allocate_new_orderId();
    void create_order(type_userid cur_user, Order order_info);
    void create_waiting_order(type_userid cur_user, Order waitingOrder_info);
    void query_order(type_userid cur_user);
    ReturnMode refund_ticket(type_userid cur_user, int num);
};

#endif //TICKET_SYSTEM_2024_ORDER_HPP
