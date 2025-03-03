#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "order.h"
#include <queue>
#include <map>
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iostream>

class OrderQueue
{
private:
    std::queue<std::unique_ptr<OrderBase>> orderQueue;
    std::mutex queueMutex;
    std::condition_variable cv;

public:
    void push(std::unique_ptr<OrderBase> order)
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        orderQueue.push(std::move(order));
        cv.notify_one();
    }

    std::unique_ptr<OrderBase> pop()
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this]
                { return !orderQueue.empty(); });
        auto order = std::move(orderQueue.front());
        orderQueue.pop();
        return order;
    }
};

class OrderBook
{
private:
    std::map<double, std::deque<std::unique_ptr<LimitOrder>>> buyOrders;
    std::map<double, std::deque<std::unique_ptr<LimitOrder>>> sellOrders;
    std::mutex bookMutex;

public:
    void addOrder(std::unique_ptr<LimitOrder> order)
    {
        std::lock_guard<std::mutex> lock(bookMutex);
        auto &bookSide = (order->side == Side::BUY) ? buyOrders : sellOrders;
        bookSide[order->price].push_back(std::move(order));
    }
    void executeMarketOrder(std::unique_ptr<MarketOrder> marketOrder)
    {
        std::lock_guard<std::mutex> lock(bookMutex);
        auto &bookSide = (marketOrder->side == Side::BUY) ? sellOrders : buyOrders;
        while (marketOrder->quantity > 0 && !bookSide.empty())
        {
            auto bestPrice = (marketOrder->side == Side::BUY) ? bookSide.begin() : std::prev(bookSide.end());
            auto &orderQueue = bestPrice->second;
            auto &limitOrder = orderQueue.front();
            int tradeQuantity = std::min(marketOrder->quantity, limitOrder->quantity);

            // std::string curr_book = printOrderBook();
            // std::string tradeLog = std::format("[TRADE] Market Order {} matched with Limit Order {} at Price: {:.2f} | Quantity: {}\n\n BOOK: {}\n",
            //                                    marketOrder->orderId, limitOrder->orderId, bestPrice->first, tradeQuantity, curr_book);
            std::string tradeLog = std::format("[TRADE] Market Order {} matched with Limit Order {} at Price: {:.2f} | Quantity: {}\n",
                                               marketOrder->orderId, limitOrder->orderId, bestPrice->first, tradeQuantity);
            {
                static std::mutex coutMutex;
                std::lock_guard<std::mutex> coutLock(coutMutex);
                std::cout << tradeLog;
            }
            marketOrder->quantity -= tradeQuantity;
            limitOrder->quantity -= tradeQuantity;
            if (limitOrder->quantity == 0)
                orderQueue.pop_front();
            if (orderQueue.empty())
                bookSide.erase(bestPrice->first);
        }
    }
    void matchOrders()
    {
        std::lock_guard<std::mutex> lock(bookMutex);
        while (!buyOrders.empty() && !sellOrders.empty())
        {
            auto bestBuy = buyOrders.rbegin();
            auto bestSell = sellOrders.begin();
            if (bestBuy->first < bestSell->first)
                break;
            auto &buyQueue = bestBuy->second;
            auto &sellQueue = bestSell->second;
            auto &buyOrder = buyQueue.front();
            auto &sellOrder = sellQueue.front();
            int tradeQuantity = std::min(buyOrder->quantity, sellOrder->quantity);
            double tradePrice = (buyOrder->timestamp < sellOrder->timestamp) ? buyOrder->price : sellOrder->price;

            // std::string curr_book = printOrderBook();
            // std::string tradeLog = std::format("[TRADE] Order {} (BUY) matched with Order {} (SELL) at Price: {:.2f} | Quantity: {}\n\n BOOK: {}\n",
            //                                    buyOrder->orderId, sellOrder->orderId, tradePrice, tradeQuantity, curr_book);
            std::string tradeLog = std::format("[TRADE] Order {} (BUY) matched with Order {} (SELL) at Price: {:.2f} | Quantity: {}\n",
                                               buyOrder->orderId, sellOrder->orderId, tradePrice, tradeQuantity);
            {
                static std::mutex coutMutex;
                std::lock_guard<std::mutex> coutLock(coutMutex);
                std::cout << tradeLog;
            }

            buyOrder->quantity -= tradeQuantity;
            sellOrder->quantity -= tradeQuantity;
            if (buyOrder->quantity == 0)
                buyQueue.pop_front();
            if (sellOrder->quantity == 0)
                sellQueue.pop_front();
            if (buyQueue.empty())
                buyOrders.erase(bestBuy->first);
            if (sellQueue.empty())
                sellOrders.erase(bestSell);
        }
    }

    std::string printOrderBook()
    {

        std::string output = "\n--- Order Book ---\n";

        output += "Asks:\n";
        int count = 0;
        for (const auto &[price, orders] : sellOrders)
        {
            if (count++ >= 5)
                break;
            output += std::format("Price: {:.2f} | Quantity: {}\n", price, orders.front()->quantity);
        }

        output += "Bids:\n";
        count = 0;
        for (auto it = buyOrders.rbegin(); it != buyOrders.rend() && count < 5; ++it, ++count)
        {
            output += std::format("Price: {:.2f} | Quantity: {}\n", it->first, it->second.front()->quantity);
        }

        return output;
    }
};

#endif // ORDERBOOK_H
