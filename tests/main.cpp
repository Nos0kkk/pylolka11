#include "../pylolka11/bot.hpp"
#include <iostream>
#include <regex>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>

int main() {
    std::ifstream read("../token.json");
    nlohmann::json j;
    read >> j;
    read.close();

    lol::client bot(j["token"].get<std::string>());

    bot.on_event(lol::event::on_ready, [](){
        std::cout << "бот запущен" << std::endl;
    });

    bot.on_event(lol::event::on_message, [](const lol::message& msg){
        if (msg.bot()) return;
        std::cout << msg.guild_id() << std::endl;
        
        std::string txt = msg.content();
        std::smatch m;
        if (std::regex_match(txt, m, std::regex(R"(!лог (.*))"))) {
            std::cout << "@" << msg.author() << " использовал команду !лог" << std::endl;
            msg.channel_send("пользователь " + msg.author_username() + " сообщил \"" + m.str(1) + "\"");
        }
    });

    bot.run();
    return 0;
}