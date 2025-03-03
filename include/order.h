#ifndef ORDERS_H
#define ORDERS_H

#include <chrono>

enum class Side
{
    BUY,
    SELL
};

class OrderBase
{
public:
    virtual ~OrderBase() = default;
    virtual Side getSide() const = 0;
    virtual uint64_t getQuantity() const = 0;
};

struct LimitOrder : public OrderBase
{
    uint64_t orderId;
    Side side;
    double price;
    uint64_t quantity;
    std::chrono::steady_clock::time_point timestamp;

    LimitOrder(uint64_t id, Side s, double p, uint64_t q)
        : orderId(id), side(s), price(p), quantity(q), timestamp(std::chrono::steady_clock::now()) {}

    Side getSide() const override { return side; }
    uint64_t getQuantity() const override { return quantity; }
};

struct MarketOrder : public OrderBase
{
    uint64_t orderId;
    Side side;
    uint64_t quantity;

    MarketOrder(uint64_t id, Side s, uint64_t q) : orderId(id), side(s), quantity(q) {}

    Side getSide() const override { return side; }
    uint64_t getQuantity() const override { return quantity; }
};

#endif // ORDERS_H
