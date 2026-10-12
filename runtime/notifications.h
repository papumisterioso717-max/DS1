#pragma once
#include <cstdint>
#include <vector>
struct TitleTerminationNotification {uint32_t routine,priority;};
std::vector<TitleTerminationNotification> queryTitleTerminationNotifications(uint8_t* base);
