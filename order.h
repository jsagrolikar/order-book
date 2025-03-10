#ifndef ORDER_H
#define ORDER_H

#include <chrono>
#include <variant>
#include <concepts>

namespace order
{

    enum class Side
    {
        BUY,
        SELL
    };

    struct LimitOrder
    {
        uint64_t orderId;
        Side side;
        double price;
        uint64_t quantity;
        std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    };

    struct MarketOrder
    {
        uint64_t orderId;
        Side side;
        uint64_t quantity;
    };

    using Order = std::variant<LimitOrder, MarketOrder>;

    template <typename T>
    concept ValidOrderType = std::is_same_v<T, LimitOrder> || std::is_same_v<T, MarketOrder>;

} // namespace order

#endif // ORDER_H
