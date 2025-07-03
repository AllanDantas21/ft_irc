#pragma once

#include "client.hpp"

class Channel {
private:
    Client clients[];
    std::string channelName;


public:
    Channel(std::string name);
    ~Channel();

    bool isValidName(std::string attemptName);
}