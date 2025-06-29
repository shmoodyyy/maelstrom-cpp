#ifndef COMMON_NODE_HEADER
#define COMMON_NODE_HEADER
#include "message.h"
#include "../ext/nlohmann/json.hpp"
#include <condition_variable>
#include <queue>
#include <atomic>
#include <mutex>

class Node 
{
  using json = nlohmann::json;
public:
  Node();
  void init(std::vector<std::string>&& all_nodes, int self_index);
  void run();
  void stop();

  using callback_fn = std::function<Message(const Message&)>;
  void register_handler(MessageType type, callback_fn handler);

private:
  void dispatch_message(std::string&& raw);
  void write_response(json resposne);

private:
  std::unordered_map<MessageType, callback_fn> handler_map;

  enum NodeState : int {
    STARTING,
    RUNNING,
    SHUTDOWN,
  };
  std::atomic<NodeState>    state;
  // 4
  std::string_view          self_node_id;
  std::vector<std::string>  all_node_ids;

  std::mutex                mutex_write_response;

  struct ThreadTask {
    std::shared_ptr<Message> message;
    callback_fn invoke;
  };
  std::mutex                mutex_thread_tasks;
  std::queue<ThreadTask>    task_queue;
  std::condition_variable   queue_condition;
};

#endif
