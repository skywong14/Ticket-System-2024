//
// Created by skywa on 2024/5/26.
//
#include "order.hpp"


Order_System::Order_System() {
    order_data.initialise("order_data");
    waitingOrder_data.initialise("waitingOrder_data");
    userOrder_data.initialise("userOrder_data");
    waitingQueue_data.initialise("waitingQueue_data");
}
int Order_System::allocate_new_orderId() {
    int orderId = userOrder_data.read_other_info();
    orderId++;
    userOrder_data.write_other_info(orderId);
    return orderId;
}

void Order_System::create_order(type_userid cur_user, Order order_info) {
    OrderId orderId;
    orderId.state = 1; orderId.orderId = order_info.orderId;
    userOrder_data.insert(cur_user.to_string(), orderId);
    order_data.insert(std::to_string(orderId.orderId), order_info);
}

void Order_System::create_waiting_order(type_userid cur_user, Order waitingOrder_info) {
    OrderId orderId;
    orderId.state = 0; orderId.orderId = waitingOrder_info.orderId;
    userOrder_data.insert(cur_user.to_string(), orderId);
    waitingOrder_data.insert(std::to_string(orderId.orderId), waitingOrder_info);
    //加入waiting_order
    waitingQueue_data.insert(to_index(waitingOrder_info.date, waitingOrder_info.trainId), waitingOrder_info.orderId);
}

void Order_System::query_order(type_userid cur_user) {
    vector< OrderId > vec = userOrder_data.search_values(cur_user.to_string());
    vector< Order > vec2;
    Order order_info;
    std::cout<<vec.size()<<std::endl;
    for (int i = 0; i < vec.size(); i++){
        if (vec[i].state == 0){
            //候补
            vec2 = waitingOrder_data.search_values(std::to_string(vec[i].orderId));
            assert( vec2.size() == 1);
            order_info = vec2[0];
        } else {
            vec2 = order_data.search_values(std::to_string(vec[i].orderId));
            assert( vec2.size() == 1);
            order_info = vec2[0];
        }
        std::cout<<order_info.to_string(vec[i].state)<<std::endl;
    }
}

string Order::to_string(int state) {
    string str;
    if (state == 0) str = "[pending] ";
    else if (state == 1) str = "[success] ";
    else if (state == 2) str = "[refunded] ";
    str = str + trainId.to_string() + ' ' + startStation.to_string() + ' ' + startTime.to_string()
            + " -> " + endStation.to_string() + ' ' + endTime.to_string() + ' ' + std::to_string(unit_price) + ' ' + std::to_string(num);
    return str;
}
