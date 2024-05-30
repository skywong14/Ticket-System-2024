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
    waitingQueue_data.insert(to_index(waitingOrder_info.singlePass.date, waitingOrder_info.singlePass.trainId), waitingOrder_info.orderId);
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
    str = str + singlePass.to_string();;
    return str;
}

string Single_Pass::to_string() {
    return trainId.to_string() + ' ' + startStation.to_string() + ' ' + startTime.to_string()
    + " -> " + endStation.to_string() + ' ' + endTime.to_string() + ' ' + std::to_string(unit_price) + ' ' + std::to_string(num);
}

Single_Pass get_Single_Pass(type_time cur_date, Train_Info cur_train_info, Train_Route cur_route, int num, type_stationName staStart, type_stationName staEnd){
    Single_Pass val;
    val.trainId = cur_train_info.trainId;
    val.trainType = cur_train_info.type;
    val.num = num;
    val.startStationPos =  cur_route.search_station(staStart);
    val.endStationPos = cur_route.search_station(staEnd);
    val.startStation = cur_route.stations[val.startStationPos];
    val.endStation = cur_route.stations[val.endStationPos];

    val.date = setOffDate(cur_date, cur_train_info.startTime, cur_route.stopoverTimes[val.startStationPos], cur_route.arriveTimes[val.startStationPos]); //发车日
    val.startTime = type_time(val.date + cur_train_info.startTime + cur_route.arriveTimes[val.startStationPos] + cur_route.stopoverTimes[val.startStationPos]);
    val.endTime = type_time(val.date + cur_train_info.startTime + cur_route.arriveTimes[val.endStationPos]);
    val.unit_price = cur_route.prices[val.endStationPos] - cur_route.prices[val.startStationPos];
    return val;
}

