//
// Created by skywa on 2024/5/26.
//
#include "order.hpp"


Order_System::Order_System() {
    order_data.initialise("order_data");
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
    order_data.insert(std::to_string(orderId.orderId), waitingOrder_info);
    //加入waiting_queue
    waitingQueue_data.insert(to_index(waitingOrder_info.singlePass.date, waitingOrder_info.singlePass.trainId), waitingOrder_info.orderId);
}

void Order_System::query_order(type_userid cur_user) {
    vector< OrderId > vec = userOrder_data.search_values(cur_user.to_string());
    vector< Order > vec2;
    Order order_info;
    std::cout<<vec.size()<<std::endl;
    for (int i = vec.size() - 1; i >= 0; i--){
        vec2 = order_data.search_values(std::to_string(vec[i].orderId));
        assert( vec2.size() == 1);
        order_info = vec2[0];
        std::cout<<order_info.to_string(vec[i].state)<<std::endl;
    }
}

ReturnMode Order_System::refund_ticket(type_userid cur_user, int num) {
    extern Train_System train_system; //extern
    vector< OrderId > vec = userOrder_data.search_values(cur_user.to_string());
    if (num > vec.size()) return ReturnMode::Out_Of_Range;
    OrderId orderId = vec[vec.size() - num];
    if (orderId.state == 2){
        return ReturnMode::Invalid_Operation;
    }
    OrderId new_orderId;
    vector< Order > vec2;
    Order cur_order;
    new_orderId.state = 2;
    new_orderId.orderId = orderId.orderId;

    vec2 = order_data.search_values(std::to_string(orderId.orderId));
    assert(vec2.size() == 1);
    cur_order = vec2[0];

    if (orderId.state == 0){
        //pending
        waitingQueue_data.erase(to_index(cur_order.singlePass.date, cur_order.singlePass.trainId), orderId.orderId);
    } else if (orderId.state == 1){
        //success
        type_Seat_Info_ptr seatInfoPtr = train_system.refund_ticket(cur_order.singlePass);

        check_waitingQueue(cur_order.singlePass.date, cur_order.singlePass.trainId, seatInfoPtr);
        //change the state in userOrder_data
        //refund seats
    }

    userOrder_data.erase(cur_user.to_string(), orderId);
    userOrder_data.insert(cur_user.to_string(), new_orderId);
    return ReturnMode::Correct;
}

void Order_System::check_waitingQueue(type_time date, type_trainID trainId, type_Seat_Info_ptr seatInfoPtr) {
    extern Train_System train_system; //extern
    Seat_Info seat_info = train_system.read_Seat_Info(seatInfoPtr);
    OrderId cur_orderId;
    Order cur_order;
    int cnt;
    vector< int > waitingIds = waitingQueue_data.search_values(to_index(date, trainId));
    for (int i = 0; i < waitingIds.size(); i++){
        cur_order = order_data.search_values(std::to_string(waitingIds[i]))[0];

        cnt = 0;
        for (int j = cur_order.singlePass.startStationPos; j < cur_order.singlePass.endStationPos; j++)
            cnt = std::max(cnt, seat_info.seat_sell[j]);
        cnt = seat_info.seat_num - cnt;

        if (cnt >= cur_order.singlePass.num){
            for (int j = cur_order.singlePass.startStationPos; j < cur_order.singlePass.endStationPos; j++)
                seat_info.seat_sell[j] += cur_order.singlePass.num;
            waitingQueue_data.erase(to_index(date, trainId), waitingIds[i]);

            cur_orderId.orderId = waitingIds[i]; cur_orderId.state = 0;
            userOrder_data.erase(cur_order.userid.to_string(), cur_orderId);
            cur_orderId.state = 1; //todo 需要一个新orderId吗
            userOrder_data.insert(cur_order.userid.to_string(), cur_orderId);
        }
    }

    train_system.update_Seat_Info(seatInfoPtr, seat_info);
}

string Order::to_string(int state) {
    string str;
    if (state == 0) str = "[pending] ";
    else if (state == 1) str = "[success] ";
    else if (state == 2) str = "[refunded] ";
    str = str + singlePass.to_string() + " " + std::to_string(singlePass.num);
    return str;
}

string Single_Pass::to_string() {
    return trainId.to_string() + ' ' + startStation.to_string() + ' ' + (date + setOffTime + startTime).to_string()
    + " -> " + endStation.to_string() + ' ' + (date + setOffTime + endTime).to_string() + ' ' + std::to_string(unit_price);
}


