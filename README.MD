# pylolka11
Это тоже самое что и `lolka.cpp` но сделана на основе `lolka.py`. Для того чтобы реализовать то что я хотел за более краткие сроки, до выхода первого релиза `lolka.cpp`

# примеры
Обычная пинг-понг команда
```cpp
#include <pylolka11/bot.hpp>

int main() {
    lol::client bot("ТОКЕН");
    
    bot.on_event(lol::event::on_ready, [](){
        std::cout << "бот запущен" << std::endl;
    });

    bot.on_event(lol::event::on_message, [](const lol::message& msg){
        if (msg.bot()) return;

        if (msg.content() == "!ping") msg.channel_send("pong");
    });

    bot.run();
    return 0;
}
```

# соц. Сети
Lolka server - https://lolka.gg/5WpNoyvIK

# License
- MIT-License