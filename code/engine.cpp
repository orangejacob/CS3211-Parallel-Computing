#include "engine.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include "io.h"
#include <map>

std::mutex print_mutex;

void Engine::Accept(ClientConnection connection) {
  // std::cout << "New Thread" << std::endl;
  std::thread thread{&Engine::ConnectionThread, this,
                     std::move(connection)};
  thread.detach();
}

void Engine::ConnectionThread(ClientConnection connection) {
  while (true) {
    input input;
    switch (connection.ReadInput(input)) {
      case ReadResult::Error:
        std::cerr << "Error reading input" << std::endl;
      case ReadResult::EndOfFile:
        return;
      case ReadResult::Success:
        break;
    }

    int64_t input_time = CurrentTimestamp();
    
    // Functions for printing output actions in the prescribed format are
    // provided in the Output class:
    switch (input.type) {
      case input_cancel:
        {
          // Retrieve the instrument name by Order ID, if doesn't exist -> Reject immediately.
          std::string instrument_name = orderBook->getInstructmentByID(input.order_id);
          
          if(instrument_name.empty()){
            std::scoped_lock<std::mutex> lock(print_mutex);
            Output::OrderDeleted(input.order_id, false, input_time, CurrentTimestamp());
            break;
          }
          // Else, Retrieve order list by instrument name.
          OrderList* orderList = orderBook->getOrderList(input.order_id, instrument_name, true);
          orderList->cancelOrder(input.order_id, input_time);
          break;
        }
      case input_buy:
      case input_sell:
      {
        // Retrieve order list by instrument name.
        OrderList* orderList = orderBook->getOrderList(input.order_id, input.instrument, false);
        Order* newOrder = new Order( input.order_id, input.count, input.price,  input.type == input_sell ? sell : buy);
        // Execute Order matching against new Order.
        orderList->matchOrder(newOrder, input_time);
        break;
      }
      default:
        // Print Order Book -> modified io.h to allow input 'P'
        orderBook->printOrderBook();
        break;
    }
  }
}

// For Debugging, to see all instrument and its respective resting orders.
void OrderBook::printOrderBook(){
  std::scoped_lock<std::mutex> lock(this->orderbook_mutex);
  std::cout << "============================================" << std::endl;
  std::cout << "[Order Book]" << std::endl;
  for(auto const& [instrument_name, order_list] : this->instrument_map){
    order_list->printOrders();
  }
  std::cout << "============================================" << std::endl;
}

// For Debugging, to see all instrument and its respective resting orders.
void OrderList::printOrders(){
  std::scoped_lock<std::mutex> lock(this->instrument_mutex);
  std::cout << "[" << this->instrument << "]" << std::endl;
  for(Order* curOrder = this->s_head; curOrder != NULL; curOrder = curOrder->next){
    std::cout << "S" << " " << curOrder->ID << " " << this->instrument << " " << curOrder->price << " " << curOrder->size << std::endl;
  }
  for(Order* curOrder = this->b_head; curOrder != NULL; curOrder = curOrder->next){
    std::cout << "B" << " " << curOrder->ID << " " << this->instrument << " " << curOrder->price << " " << curOrder->size << std::endl;
  }
}


// Retrieve Instrument Name via Order ID
std::string OrderBook::getInstructmentByID(int order_id){
  // Lock Order Book.
  std::scoped_lock lock(this->orderbook_mutex);
  // Iterative Map to find order id
  std::map<int, std::string>::iterator iter = this->order_to_instrument.find(order_id);
  // The only time this function is executed, is when we are cancelling order.
  // Hence, we already can erase from the map before executing Cancel Order.
  if (iter != this->order_to_instrument.end()){
    std::string instrument_name = iter->second;
    this->order_to_instrument.erase(iter);
    return instrument_name;
  }
  return "";
}

// Cancel Order.
void OrderList::cancelOrder(int order_id, std::chrono::microseconds::rep input_time_stamp){
  // Find Order object through resting_orders from the Order List.
  std::scoped_lock<std::mutex> lock(this->instrument_mutex);
  std::map<int, Order*>::iterator iter = resting_orders.find(order_id) ;
  
  if (iter != resting_orders.end()){
    // Order exist -> perform pointer readjustments  
    Order* del_order = iter->second;
    // Order to be deleted is at the head.
    if(del_order->prev == NULL){
      if(del_order->side == buy){
        this->b_head = del_order->next;
        if(this->b_head != NULL)
          this->b_head->prev = NULL;
      }else{
        this->s_head = del_order->next;
        if(this->s_head != NULL)
          this->s_head->prev = NULL;
      }
    }else if(del_order->next == NULL){
      del_order->prev->next = NULL;
    }else{
      del_order->prev->next = del_order->next;
      del_order->next = del_order->prev;
    }
    // Erase from map and free memory.
    resting_orders.erase(iter);
    delete del_order;
    del_order = NULL;
    {
      std::scoped_lock<std::mutex> lock(print_mutex);
      Output::OrderDeleted(order_id, true, input_time_stamp, CurrentTimestamp());
    }
  }else{
      // Order doesn't exist -> either false or fufilled order.
    {
      std::scoped_lock<std::mutex> lock(print_mutex);
      Output::OrderDeleted(order_id, false, input_time_stamp, CurrentTimestamp());
    }
  }
}

// Perform matching through Doubly Linked List.
void OrderList::matchOrder(Order* new_order, std::chrono::microseconds::rep input_time_stamp){
  // Lock the order list.
  std::scoped_lock<std::mutex> lock(this->instrument_mutex);

  if(new_order->side == buy){
    // There's no sell resting order, hence just insert Buy Order.
    if(this->s_head == NULL){
      insertBuyOrder(new_order, input_time_stamp);
      return;
    }
    // Sell order are stored in ascending order -> Hence just traverse through the pointer.
    Order* tmp = NULL;
    Order* cur_order = this->s_head;
    while(cur_order != NULL && (new_order->price >= cur_order->price) && (new_order->size > 0) && (cur_order->size > 0)){
      cur_order->incrementExecuted();
      if(cur_order->size > new_order->size){
        // Update Resting Order's size.
        cur_order->setSize(cur_order->size - new_order->size);
        {
          std::scoped_lock<std::mutex> lock(print_mutex);
          Output::OrderExecuted(cur_order->ID, new_order->ID, cur_order->executedAmount, cur_order->price, new_order->size, input_time_stamp, CurrentTimestamp());
        }
        new_order->setSize(0);
        delete new_order;
      }else{
        new_order->setSize(new_order->size - cur_order->size);
        {
          std::scoped_lock<std::mutex> lock(print_mutex);
          Output::OrderExecuted(cur_order->ID, new_order->ID, cur_order->executedAmount, cur_order->price, cur_order->size, input_time_stamp, CurrentTimestamp());
        }
        tmp = cur_order;
        cur_order = cur_order->next;
        std::map<int, Order*>::iterator iter = this->resting_orders.find(tmp->ID);
        if (iter != resting_orders.end())
          resting_orders.erase(iter);
        if(tmp != NULL){
          delete tmp;
          tmp = NULL;
        }
      }
    }
    // Handle pointer.
    this->s_head = cur_order;
    if(this->s_head != NULL)
      this->s_head->prev = NULL;
  }else{
    // There's no buy resting order, hence just insert sell Order.
    if(this->b_head == NULL){
      insertSellOrder(new_order, input_time_stamp);
      return;
    }

    Order* tmp = NULL;
    Order* cur_order = this->b_head;
    // Buy order are stored in Descending order -> Hence just traverse through the pointer.
    while(cur_order != NULL && (cur_order->price >= new_order->price) &&(new_order->size > 0) && (cur_order->size > 0)){
       cur_order->incrementExecuted();
      if(cur_order->size > new_order->size){
        // Update Resting Order's size.
        cur_order->setSize(cur_order->size - new_order->size);
        {
          std::scoped_lock<std::mutex> lock(print_mutex);
          Output::OrderExecuted(cur_order->ID, new_order->ID, cur_order->executedAmount, cur_order->price, new_order->size, input_time_stamp, CurrentTimestamp());
        }
        new_order->setSize(0);
        delete new_order;
      }else{
        // Update incoming order size.
        new_order->setSize(new_order->size - cur_order->size);
        {
          std::scoped_lock<std::mutex> lock(print_mutex);
          Output::OrderExecuted(cur_order->ID, new_order->ID, cur_order->executedAmount, cur_order->price, cur_order->size, input_time_stamp, CurrentTimestamp());
        }
        tmp = cur_order;
        cur_order = cur_order->next;
        // Delete the resting order from memory and map as it has been fufilled.
        std::map<int, Order*>::iterator iter = this->resting_orders.find(tmp->ID);
        if (iter != resting_orders.end())
          resting_orders.erase(iter);
        if(tmp != NULL){
          delete tmp;
          tmp = NULL;
        }
      }
    }
    // Handle pointer.
    this->b_head = cur_order;
    if(this->b_head != NULL)
      this->b_head->prev = NULL;
  }

  if(new_order->size > 0){
    if(new_order->side == buy)
      insertBuyOrder(new_order, input_time_stamp);
    else
      insertSellOrder(new_order, input_time_stamp);
  }
}

void OrderList::insertBuyOrder(Order* new_order, std::chrono::microseconds::rep input_time_stamp){
  // Insert into resting order map.
  this->resting_orders.insert(std::pair<int, Order*>(new_order->ID, new_order));

  if(this->b_head == NULL){
    this->b_head = new_order;
  }else if(new_order->price > this->b_head->price){
    // Biggest Buy Order.
    new_order->next = this->b_head;
    new_order->next->prev = new_order;
    this->b_head = new_order;
  }else{

    // Buy Linked List is sorted by Descending order.
    Order* cur_order = this->b_head;
    // Find insertion spot.
    for(;cur_order->next != NULL && cur_order->next->price >= new_order->price; cur_order = cur_order->next);
    // The smallest buy order.
    if(cur_order->next == NULL){
      cur_order->next = new_order;
      new_order->prev = cur_order;
    }else{
      new_order->next = cur_order->next;
      new_order->next->prev = new_order;
      cur_order->next = new_order;
      new_order->prev = cur_order;
    }
  }
  {
    std::scoped_lock<std::mutex> lock(print_mutex);
    Output::OrderAdded(new_order->ID, this->instrument.c_str(), new_order->price, new_order->size, false, input_time_stamp, CurrentTimestamp());
  }
}


void OrderList::insertSellOrder(Order* new_order, std::chrono::microseconds::rep input_time_stamp){
  // Insert into resting order map.
  this->resting_orders.insert(std::pair<int, Order*>(new_order->ID, new_order));

  if(this->s_head == NULL){
    this->s_head = new_order;
  }else if(this->s_head->price > new_order->price){
    // Lowest Sell Order
    new_order->next = this->s_head;
    new_order->next->prev = new_order;
    this->s_head = new_order;
  }else{
    // Sell Linked List is sorted by Ascending order.
    Order* cur_order = this->s_head;
    // Find insertion spot.
    for(;cur_order->next != NULL && new_order->price >= cur_order->next->price; cur_order = cur_order->next);
    // The smallest buy order.
    if(cur_order->next == NULL){
      cur_order->next = new_order;
      new_order->prev = cur_order;
    }else{
      new_order->next = cur_order->next;
      new_order->next->prev = new_order;
      cur_order->next = new_order;
      new_order->prev = cur_order;
    }
  }
  {
    std::scoped_lock<std::mutex> lock(print_mutex);
    Output::OrderAdded(new_order->ID, this->instrument.c_str(), new_order->price, new_order->size, true, input_time_stamp, CurrentTimestamp());
  }
}

// Retrieve OrderList for a specific instrument.
OrderList* OrderBook::getOrderList(int order_id, std::string instrument_name, bool is_cancel){
  // Lock the Order Book.
  std::scoped_lock<std::mutex> lock(this->orderbook_mutex);
  auto it = this->instrument_map.find(instrument_name);

  if(!is_cancel)
    // Operation is used for Matching Order, hence record down the Order ID <-> Instrument Name pair.
    // Will be used for cancelling orders.
    this->order_to_instrument.insert(std::pair<int, std::string>(order_id, instrument_name));
  
  if (it == this->instrument_map.end()){
    // There's no order list for this instrument yet -> Create one.
    OrderList* newOrderList = new OrderList(instrument_name);
    this->instrument_map.insert(std::pair<std::string, OrderList*>(instrument_name, newOrderList));
    return newOrderList;
  }
  return it->second;
}