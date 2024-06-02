//
// Created by skywa on 2024/5/25.
//
#include "user.hpp"

bool User_system::logged_in(User_info &ret, type_userid userid) {
    if (user_online.find(userid) != user_online.end()){
        ret = user_online[userid];
        return true;
    }
    return false;
}


bool User_system::login(type_userid userid_, User_info info_) {
    user_online.insert(std::make_pair(userid_, info_));
    return true;
}

bool User_system::logout(type_userid userid_) {
    auto it = user_online.find(userid_);
    user_online.erase(it);
    return true;
}

void User_system::update_user_logged_in(type_userid userid_, User_info new_info_) {
    user_online[userid_] = new_info_;
}

//in data_base

bool User_system::User_exist(type_userid userid_) {
    vector<User_info> val = user_data.search_values( userid_.to_string() );
    assert(val.size() <= 1); //for debug
    return !val.empty();
}


bool User_system::get_User_info(User_info &ret, type_userid userid_) {
    vector<User_info> val = user_data.search_values( userid_.to_string() );
    assert(val.size() <= 1); //for debug
    if (!val.empty()){
        ret = val[0];
        return true;
    }
    return false;
}

bool User_system::delete_User(type_userid userid_, User_info info_) {
    user_data.erase(userid_.to_string(), info_);
    return false;
}

bool User_system::add_User(type_userid userid_, User_info info_) {
    user_data.insert(userid_.to_string(), info_);
    return false;
}

bool User_system::modify_User(type_userid userid_, User_info pre_info_, User_info new_info_) {
    delete_User(userid_, pre_info_);
    add_User(userid_, new_info_);
    return false;
}

User_system::User_system():user_online() {
    user_data.initialise("user");
}

User_system::~User_system() {
    user_data.pop_all_buffer();
    user_online.clear();
}

bool User_system::empty() {
    return user_data.empty();
}






