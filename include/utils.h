#ifndef UTILS_H
#define UTILS_H

#include "orderbook.h"
#include <random>

void orderProcessor(OrderBook &book, OrderQueue &queue)
{
    while (true)
    {
        auto order = queue.pop();
        if (auto *limitOrder = dynamic_cast<LimitOrder *>(order.get()))
        {
            book.addOrder(std::make_unique<LimitOrder>(*limitOrder));
            book.matchOrders();
        }
        else if (auto *marketOrder = dynamic_cast<MarketOrder *>(order.get()))
        {
            book.executeMarketOrder(std::make_unique<MarketOrder>(*marketOrder));
        }
    }
}

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_real_distribution<double> BpriceDist(95.0, 101.0);
std::uniform_real_distribution<double> ApriceDist(99.0, 105.0);
std::uniform_int_distribution<uint64_t> qtyDist(1, 100);
std::uniform_int_distribution<int> sideDist(0, 1);

void pushRandomLimitOrder(OrderQueue &orderQueue, uint64_t id)
{
    Side side = (sideDist(gen) == 0) ? Side::BUY : Side::SELL;
    double price = (side == Side::BUY) ? BpriceDist(gen) : ApriceDist(gen);
    uint64_t quantity = qtyDist(gen);
    orderQueue.push(std::make_unique<LimitOrder>(id, side, price, quantity));
}

void pushRandomMarketOrder(OrderQueue &orderQueue, uint64_t id)
{
    Side side = (sideDist(gen) == 0) ? Side::BUY : Side::SELL;
    uint64_t quantity = qtyDist(gen);
    orderQueue.push(std::make_unique<MarketOrder>(id, side, quantity));
}

#endif // UTILS_H
