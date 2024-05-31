// Created by skywa on 2024/5/25.
#include <iostream>
#include <cstdio>
#include "user.hpp"
#include "train.hpp"
#include "../tools/MyTools.hpp"
#include "order.hpp"

vector<string> cur_tokens, other_tokens;
string arguments[26];
Command_Head com_head;
User_system user_system;
Train_System train_system;
Order_System order_system;

CmpSinglePass_Time cmpSinglePass_Time;
CmpSinglePass_Cost cmpSinglePass_Cost;

int main(){
//    freopen("","r",stdin);
//    freopen("MyTest.txt","r",stdin);
//    freopen("MyAnswer.txt","w",stdout);
    User_info cur_user_info, other_user_info, tmp_user_info;
    Train_Info cur_train_info, other_train_info;
    type_time cur_time, other_time;
    Train_Route cur_route;
    type_stationName cur_station, other_station;
    vector<Station> stations;
    vector<Single_Pass> trains;
    DayTicket day_ticket;
    Order order_info;

    type_trainID cur_train_id;
    bool pending;
    bool the_first_user = user_system.empty();
    ReturnMode ret_mode;
    int tmp_num, posStart, posEnd;

    bool system_open = true;
    while (system_open){
        cur_tokens = get_tokens();
        if (cur_tokens.empty()) continue;
        if (get_Command_Head(com_head, cur_tokens) != ReturnMode::Correct){
             continue;
        }
        if (get_arguments(arguments, cur_tokens) != ReturnMode::Correct){
            continue;
        }

        //time_stamp: com_head.first; command: com_head.second;
        //token_pair: -<key> and <argument>

        std::cout<<'['<<com_head.first<<"] ";
        //保证所有指令输入格式均合法
        switch (com_head.second) {
            case Command_Name::add_user:
                //-c -u -p -n -m -g
                ret_mode = ReturnMode::Other_Error;

                if (the_first_user){
                    cur_user_info.privilege = 10;
                    cur_user_info.mailAddr = arguments['m' - 'a'];
                    cur_user_info.realname = arguments['n' - 'a'];
                    cur_user_info.password = arguments['p' - 'a'];
                    cur_user_info.userid = arguments['u' - 'a'];

                    user_system.add_User(cur_user_info.userid, cur_user_info);
                    the_first_user = false;
                    ret_mode = ReturnMode::Correct;

                } else if (user_system.logged_in(cur_user_info, type_userid(arguments['c' - 'a']))){ //已登陆
                    ret_mode = ReturnMode::Wrong_Value;
                    if (cur_user_info.privilege > std::stoi(arguments['g' - 'a']) //权限更高
                        && !user_system.User_exist(arguments['u' - 'a']) ) { // 新增用户不存在

                        other_user_info.privilege = std::stoi(arguments['g' - 'a']);
                        other_user_info.mailAddr = arguments['m' - 'a'];
                        other_user_info.realname = arguments['n' - 'a'];
                        other_user_info.password = arguments['p' - 'a'];
                        other_user_info.userid = arguments['u' - 'a'];
                        user_system.add_User(other_user_info.userid, other_user_info);
                        ret_mode = ReturnMode::Correct;
                    }
                }

                if (ret_mode == ReturnMode::Correct) std::cout<<0<<std::endl;
                else std::cout<<-1<<std::endl;
                break;
            case Command_Name::login:
                //-u -p
                ret_mode = ReturnMode::Other_Error;

                if (!user_system.logged_in(other_user_info, arguments['u' - 'a']) //未登录
                        && user_system.get_User_info(cur_user_info, arguments['u' - 'a'])){ //user exist
                    ret_mode = ReturnMode::Wrong_Value;
                    if (cur_user_info.password == arguments['p' - 'a']){ //密码正确
                        user_system.login(cur_user_info.userid, cur_user_info);
                        ret_mode = ReturnMode::Correct;
                    }
                }

                if (ret_mode == ReturnMode::Correct) std::cout<<0<<std::endl;
                else std::cout<<-1<<std::endl;
                break;
            case Command_Name::logout:
                //-u
                ret_mode = ReturnMode::Other_Error;

                if (user_system.logged_in(other_user_info, arguments['u' - 'a'])){
                    user_system.logout(arguments['u' - 'a']);
                    ret_mode = ReturnMode::Correct;
                }

                if (ret_mode == ReturnMode::Correct) std::cout<<0<<std::endl;
                else std::cout<<-1<<std::endl;
                break;
            case Command_Name::query_profile:
                // -c -u
                ret_mode = ReturnMode::Other_Error;
                if (user_system.logged_in(cur_user_info, arguments['c' - 'a'])
                    && user_system.get_User_info(other_user_info, arguments['u' - 'a'])){
                    if (cur_user_info.privilege > other_user_info.privilege || arguments['c' - 'a'] == arguments['u' - 'a']){
                        other_user_info.output();
                        ret_mode = ReturnMode::Correct;
                    }
                }

                if (ret_mode != ReturnMode::Correct) std::cout<<-1<<std::endl;
                break;
            case Command_Name::modify_profile:
                // -c -u (-p) (-n) (-m) (-g)
                ret_mode = ReturnMode::Other_Error;
                if (user_system.logged_in(cur_user_info, arguments['c' - 'a'])
                    && user_system.get_User_info(other_user_info, arguments['u' - 'a'])){
                    if (cur_user_info.userid == other_user_info.userid){ //修改自己
                        if (arguments['g' - 'a'].empty() || cur_user_info.privilege > std::stoi(arguments['g' - 'a'])){ //权限要求
                            if (!arguments['p' - 'a'].empty())
                                other_user_info.password = arguments['p' - 'a'];
                            if (!arguments['n' - 'a'].empty())
                                other_user_info.realname = arguments['n' - 'a'];
                            if (!arguments['m' - 'a'].empty())
                                other_user_info.mailAddr = arguments['m' - 'a'];
                            if (!arguments['g' - 'a'].empty())
                                other_user_info.privilege = std::stoi(arguments['g' - 'a']);

                            user_system.modify_User(cur_user_info.userid, cur_user_info, other_user_info);
                            user_system.update_user_logged_in(cur_user_info.userid, other_user_info);

                            ret_mode = ReturnMode::Correct;
                        }
                    } else { //修改别的用户
                        if (arguments['g' - 'a'].empty()) tmp_num = 0;
                        else tmp_num = std::stoi(arguments['g' - 'a']);
                        if (cur_user_info.privilege > std::max(tmp_num, other_user_info.privilege )){ //权限要求
                            tmp_user_info = other_user_info;
                            if (!arguments['p' - 'a'].empty())
                                other_user_info.password = arguments['p' - 'a'];
                            if (!arguments['n' - 'a'].empty())
                                other_user_info.realname = arguments['n' - 'a'];
                            if (!arguments['m' - 'a'].empty())
                                other_user_info.mailAddr = arguments['m' - 'a'];
                            if (!arguments['g' - 'a'].empty())
                                other_user_info.privilege = std::stoi(arguments['g' - 'a']);

                            user_system.modify_User(other_user_info.userid, tmp_user_info, other_user_info);
                            if (user_system.logged_in(tmp_user_info, other_user_info.userid)){
                                user_system.update_user_logged_in(other_user_info.userid, other_user_info);
                            }

                            ret_mode = ReturnMode::Correct;
                        }
                    }
                }

                if (ret_mode == ReturnMode::Correct)
                    other_user_info.output();
                else std::cout<<-1<<std::endl;
                break;
            case Command_Name::add_train:
                // -i -n -m -s -p -x -t -o -d -y
                ret_mode = ReturnMode::Correct;
                if (train_system.exist_trainId(other_train_info, arguments['i' - 'a'])){
                    ret_mode = ReturnMode::Wrong_Value;
                    std::cout<<-1<<std::endl;
                    break;
                }

                //searNum
                cur_route.num = std::stoi(arguments['n' - 'a']);
                //-s stations
                other_tokens = split_by_vertical_bar(arguments['s' - 'a']);
                assert(cur_route.num == other_tokens.size()); // for debug only
                cur_route.write_info('s', other_tokens);

                //-o stopoverTimes
                other_tokens = split_by_vertical_bar(arguments['o' - 'a']);
                if (other_tokens[0] == "_") other_tokens.clear();
                cur_route.write_info('o', other_tokens);

                //-t travelTimes
                other_tokens = split_by_vertical_bar(arguments['t' - 'a']);
                cur_route.write_info('t', other_tokens);

                //-p prices
                other_tokens = split_by_vertical_bar(arguments['p' - 'a']);
                cur_route.write_info('p', other_tokens);

                //calc arriveTimes
                cur_route.calc_arrive();

                //-d saleDate
                other_tokens = split_by_vertical_bar(arguments['d' - 'a']);
                assert(other_tokens.size() == 2);
                cur_train_info.BeginDate = type_time(other_tokens[0]);
                cur_train_info.EndDate = type_time(other_tokens[1]);

                cur_train_info.trainId = arguments['i' - 'a'];
                cur_train_info.stationNum = cur_route.num;
                cur_train_info.seatNum = std::stoi(arguments['m' - 'a']);
                cur_train_info.startTime = type_time(arguments['x' - 'a']);
                cur_train_info.routePtr = train_system.new_Train_Route(cur_route);
                cur_train_info.type = arguments['y' - 'a'][0];
                cur_train_info.released = false;
                assert(arguments['y' - 'a'].size() == 1); //for debug only

                train_system.add_train(arguments['i' - 'a'], cur_train_info);

                std::cout<<0<<std::endl;
                break;
            case Command_Name::delete_train:
                // -i
                ret_mode = ReturnMode::Wrong_Value;

                cur_train_id = arguments['i' - 'a'];
                if (train_system.exist_trainId(cur_train_info, cur_train_id)){
                    ret_mode = ReturnMode::Invalid_Operation;
                    if (!cur_train_info.released){
                        train_system.delete_train(cur_train_id, cur_train_info);
                        ret_mode = ReturnMode::Correct;
                    }
                }

                if (ret_mode == ReturnMode::Correct) std::cout<<0<<std::endl;
                else std::cout<<-1<<std::endl;
                break;
            case Command_Name::release_train:
                // -i
                ret_mode = ReturnMode::Other_Error;
                cur_train_id = arguments['i' - 'a'];

                if (train_system.exist_trainId(cur_train_info, cur_train_id)){
                    ret_mode = ReturnMode::Invalid_Operation;
                    if (!cur_train_info.released){
                        train_system.release_train(cur_train_id, cur_train_info);
                        ret_mode = ReturnMode::Correct;
                    }
                }
//                output_ReturnMode(ret_mode, com_head.first, cur_train_id.to_string());
                if (ret_mode == ReturnMode::Correct) std::cout<<0<<std::endl;
                else std::cout<<-1<<std::endl;
                break;
            case Command_Name::query_train:
                // -i -d
                ret_mode = ReturnMode::Other_Error;
                cur_train_id = arguments['i' - 'a'];
                cur_time = type_time( arguments['d' - 'a'] ); //date

                ret_mode = train_system.query_train(cur_time, cur_train_id);

                if (ret_mode != ReturnMode::Correct) std::cout<<-1<<std::endl;
                break;
            case Command_Name::query_ticket:
                // -s -t -d (-p time)
                if (arguments['p' - 'a'].empty()) arguments['p' - 'a'] = "time";

                cur_time = type_time(arguments['d' - 'a']);
                cur_station = arguments['s' - 'a'];
                other_station = arguments['t' - 'a'];

                trains = train_system.pass_by_trains(cur_time, cur_station, other_station);

                if (arguments['p' - 'a'] == "time"){
                    trains = sort_Single_Pass(trains, cmpSinglePass_Time);
                } else {
                    trains = sort_Single_Pass(trains, cmpSinglePass_Cost);
                }

                std::cout<<trains.size()<<std::endl;

                for (int i = 0; i < trains.size(); i++){
                    tmp_num = train_system.maximum_seats(trains[i].date, trains[i].trainId, trains[i].startStationPos, trains[i].endStationPos);
                    std::cout<<trains[i].to_string()<<' '<< tmp_num <<std::endl;
                }

                break;
            case Command_Name::query_transfer:

                std::cout<<0<<std::endl;

                break;
            case Command_Name::buy_ticket:
                //-u -i -d -n -f -t (-q false)
                ret_mode = ReturnMode::Other_Error;

                if (arguments['q' - 'a'] == "true") pending = true; else pending = false;
                cur_station = arguments['f' - 'a'];
                other_station = arguments['t' - 'a'];
                cur_time = type_time(arguments['d' - 'a']); //这里的日期是列车从 -f 出发的日期
                cur_train_id = arguments['i' - 'a'];

                if (user_system.logged_in(cur_user_info, arguments['u' - 'a'])
                    && train_system.exist_trainId(cur_train_info, cur_train_id)
                    && cur_train_info.released){
                    //用户已登录，列车存在，且已发布
                    cur_route = train_system.read_Train_Route(cur_train_info.routePtr);

                    posStart = cur_route.search_station(cur_station);
                    posEnd = cur_route.search_station(other_station);

                    if (posStart >= 0 && posEnd >= 0){ //站台存在
                        other_time = setOffDate(cur_time, cur_train_info.startTime, cur_route.stopoverTimes[posStart], cur_route.arriveTimes[posStart]); //发车日

                        if (std::stoi(arguments['n' - 'a']) <= cur_train_info.seatNum //不超过最大Seat数
                            && train_system.exist_DayTicket(day_ticket, other_time, cur_train_id)){
                            //列车时间正确，存在对应日子的DayTicket
                            //other_time为始发日期
                            //抵达时间 other_time + cur_train_info.startTime + cur_route.arriveTimes[posStart]
                            //离开始发站时间 other_time + cur_train_info.startTime + cur_route.arriveTimes[posStart] + cur_route.stopoverTimes[posStart]

                            tmp_num = train_system.maximum_seats(other_time, cur_train_id, posStart, posEnd);
//                        tmp_num = train_system.maximum_seats(cur_route, day_ticket, cur_station, other_station);

                            order_info.orderId = order_system.allocate_new_orderId(); //可能会产生空着的
                            order_info.userid = cur_user_info.userid;

                            order_info.singlePass = get_Single_Pass(cur_time, cur_train_info, cur_route,
                                                                    std::stoi(arguments['n' - 'a']), cur_station, other_station);

                            if (tmp_num >= std::stoi(arguments['n' - 'a']) ){
                                //buy ticket and create order
                                train_system.buy_ticket(day_ticket, posStart, posEnd, std::stoi(arguments['n' - 'a']));

                                order_system.create_order(cur_user_info.userid, order_info);

                                std::cout<<(order_info.singlePass.unit_price * 1ll) * (order_info.singlePass.num * 1ll)<<std::endl;
                                ret_mode = ReturnMode::Correct;
                            } else {
                                ret_mode = ReturnMode::Wrong_Value;
                                if (pending){
                                    //add to queue and create order
                                    order_system.create_waiting_order(cur_user_info.userid, order_info);
                                    std::cout<<"queue"<<std::endl;
                                    ret_mode = ReturnMode::Correct;
                                }
                            }
                        }
                    }
                }
                if (ret_mode != ReturnMode::Correct) std::cout<<-1<<std::endl;
                break;
            case Command_Name::query_order:
                // -u
                ret_mode = ReturnMode::Other_Error;

                if (user_system.logged_in(cur_user_info, arguments['u' - 'a'])){
                    order_system.query_order(cur_user_info.userid);
                    ret_mode = ReturnMode::Correct;
                }

                if (ret_mode != ReturnMode::Correct) std::cout<<-1<<std::endl;
                break;
            case Command_Name::refund_ticket:
                //-u (-n 1)
                ret_mode = ReturnMode::Other_Error;
                if (arguments['n' - 'a'].empty()) arguments['n' - 'a'] = "1";
                tmp_num = std::stoi(arguments['n' - 'a']);
                if (user_system.logged_in(cur_user_info, arguments['u'- 'a'])){
                    ret_mode = order_system.refund_ticket(arguments['u'- 'a'], tmp_num);
                }

                if (ret_mode == ReturnMode::Correct) std::cout<<0<<std::endl;
                else std::cout<<-1<<std::endl;
                break;
            case Command_Name::clean:

                break;
            case Command_Name::exit:
                if (com_head.first == 100000){
                    type_time date("07-17"); type_trainID id("LeavesofGrass");
                    train_system.exist_trainId(cur_train_info,id);
                    DayTicket dayTicket;train_system.exist_DayTicket(dayTicket, date,id);
                    Seat_Info info = train_system.read_Seat_Info(dayTicket.seatInfo_ptr);
                    std::cout<<info.seat_num<<":   ";
                    for (int i = 0; i < cur_train_info.stationNum; i++)
                        std::cout<<info.seat_sell[i]<<',';std::cout<<std::endl;
                } // for debug only

                std::cout<<"bye"<<std::endl;
                system_open = false;
                break;
        }
//        output_ReturnMode(ret_mode, com_head.first);
    }
    return 0;
}