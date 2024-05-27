//
// Created by skywa on 2024/5/25.
//
#include "user.hpp"

bool User_system::logged_in(User_info &ret, type_userid userid) {
    return false;
}

bool User_system::get_User_info(User_info &ret, type_userid userid_) {
    return false;
}

bool User_system::login(type_userid userid_, User_info info_) {
    return false;
}

bool User_system::logout(type_userid userid_) {
    return false;
}

bool User_system::delete_User(type_userid userid_) {
    return false;
}

bool User_system::add_User(type_userid userid_, User_info info_) {

    return false;
}

bool User_system::modify_User(type_userid userid_, User_info pre_info_, User_info new_info_) {
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

bool User_system::User_exist(type_userid userid_) {
    return false;
}

void User_system::update_user_logged_in(type_userid userid_, User_info new_info_) {

}





