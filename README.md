# Order Book
MPCS 51044 final project

To compile, I've been running:

    clang++ -std=c++20 -pthread main.cpp -o order-book

This project implements an order book for matching buy and sell orders in a market. The system supports two types of orders: limit orders and market orders. Orders are concurrently produced by producer threads and processed by consumer threads, with thread synchronization to ensure safe concurrent access to the order book / queue.

## Brief explanation of features

### Files
order.h contains the definitions for the Order variant and Market/Limit orders.

book.h contains the definitions for the OrderQueue and OrderBook classes.

utils.h contains definitions/specifications on how the threads should be run along with specifications for the random generation of orders. There are some constants defined at the top of this file that parametrize how long the threads should run as well as how often the order book should be logged to the console.

main.cpp initializes the threads to create/process orders. Instead of having main be an indefinite process, I set parameters in utils.h that tell the producer threads to stop producing after some number of orders, somewhat arbitrarily. The stop condition can be modified, but for this purpose, I felt like this was fine.

sample_output.txt is just the console log of one run of the executable created for your reference.

### Design choices/ideas
Limit orders: Buy or sell limit orders are placed at a specific price level + quantity and are executed at the level or better.

Market orders: Buy or sell market orders are placed with quantity and execute against the best available limit order in book.

I chose to make orders a std::variant with relevant polymorphisms done with std::visit or templatizing (depending on function). I originally considered using a base + derived classes, but I felt that since I was only supporting two types of orders, this implementation is cleaner. If I wanted to have users add new order types, I would have definitely reconsidered this, but for now, I think this implementation is nice and definitely less clunky than the alternative.

Producer threads simulate users generating orders, and consumer threads process and match these orders concurrently.

### Thread safety
I implemented a both a thread-safe OrderQueue and OrderBook class to handle the producer and consumer threads working with them concurrently. Wherever convenient, I used atomics as well. Both classes use a mutex to synchronize access to shared data structures (buy/sell orders).

### Generation / processing of orders:
Producer threads randomly generate limit and market orders with varying prices and quantities. I use std::random and some constructed distributions to generate these orders, trying to simulate what an actual market would look like.

Consumer threads process/match orders by taking orders from the queue and using the book.

### Console output
I have commented out several statements that log trades / executions if you would like to see more, but for now, I log a snapshot of the order book using the printOrderBook() method in the OrderBook class every 1,000,000 orders. If you'd like to add them in, here they are:

book.h, line 129:

    std::cout << std::format("Matching Orders: Best Buy {:.2f} Best Sell {:.2f}\n", bestBuy->first, bestSell->first);

book.h, line 112:

    std::cout << std::format("Executing Market Order {} Side: {}\n", marketOrder.orderId, (marketOrder.side == Side::BUY ? "BUY" : "SELL"));

book.h, line 101:

    std::cout << std::format("Adding Limit Order {} Side: {}\n", order.orderId, (order.side == Side::BUY ? "BUY" : "SELL"));

book.h, line 63:

    std::string tradeLog = std::format("[TRADE] Order {} (BUY) matched with Order {} (SELL) at Price: {:.2f} | Quantity: {}\n", order.orderId, limitOrder.orderId, bestPriceIt->first, tradeQuantity); std::cout << tradeLog;
        
