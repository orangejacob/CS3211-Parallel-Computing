// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <chrono>
#include "io.h"
#include <map>
#include <string>
#include <vector>
#include <mutex>

enum OrderType {buy, sell};

class Order{
  public:
    Order *prev = NULL;
    Order *next = NULL;
    int ID;
    int size;
    int price;
    OrderType side;
    int executedAmount;
    void setSize(int newSize){ size = newSize; }
    void incrementExecuted(){ executedAmount += 1;}
    Order(int _id, int _size, int _price, OrderType _type): ID(_id), size(_size), price(_price), side(_type), executedAmount(0){}
};

class OrderList{
  private:
    Order* b_head = NULL; // Sorted Descending (Buy)
    Order* s_head = NULL; // Sorted Ascending (Sell)
    std::string instrument;
    std::mutex instrument_mutex;
    std::map<int, Order*> resting_orders;
  public:
    void printOrders();
    void matchOrder(Order* order, std::chrono::microseconds::rep input_time_stamp);
    void cancelOrder(int order_ID, std::chrono::microseconds::rep input_time_stamp);
    void insertSellOrder(Order* order, std::chrono::microseconds::rep input_time_stamp);
    void insertBuyOrder(Order* order, std::chrono::microseconds::rep input_time_stamp);
    OrderList(std::string instrument_name): instrument(instrument_name){};
};

class OrderBook{
  private:
    std::mutex orderbook_mutex;
    std::map<int, std::string> order_to_instrument;
    std::map<std::string, OrderList*> instrument_map;
  public:
    void printOrderBook();
    std::string getInstructmentByID(int order_ID);
    OrderList* getOrderList(int order_id, std::string instrument_name, bool is_cancel);
};

class Engine {

  void ConnectionThread(ClientConnection);

 public:
    OrderBook* orderBook;
    Engine(){ orderBook = new OrderBook();}
    void Accept(ClientConnection);
};

inline static std::chrono::microseconds::rep CurrentTimestamp() noexcept {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

#endif
