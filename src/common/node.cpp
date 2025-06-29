#include "node.h"
#include "common/message.h"
#include "ext/nlohmann/json.hpp"
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

Node::Node(int num_workers)
  : worker_count(num_workers)
{}

void Node::init(std::vector<std::string>&& all_nodes, int self_index)
{
  if (!all_node_ids.empty() || !self_node_id.empty()) {
    std::clog << "[❌][SYS] received 'init' message after node already initialized\n";
    return;
  }
  // TODO: add middleware to be pissy about unrecognized node id references
  all_node_ids = std::move(all_nodes);
  self_node_id = all_node_ids.at(self_index);
    std::clog << "[✅][SYS] node initialized\n";
}

void Node::run()
{
  std::vector<std::thread> worker_pool(worker_count);
  for (std::thread& worker : worker_pool) {
    auto worker_fn = ([this] {
      while (true) {
        std::unique_lock queue_lock(mutex_thread_tasks);
        queue_condition.wait(queue_lock, [this]{ return state == SHUTDOWN || !task_queue.empty(); });
        if (state == SHUTDOWN && task_queue.empty())
          return;
        ThreadTask task = std::move(task_queue.front());
        task_queue.pop();
        queue_lock.unlock();

        std::clog << "[" << std::this_thread::get_id() << "][⚒️][JOB] invoking '" << message_type_to_string(task.message->type) 
                  << "' handler on message " << task.message->as_json() << '\n';
        Message response = task.invoke(*task.message);
        if (response.type != INVALID) {
          std::unique_lock lock(mutex_write_response);
          std::clog << "[" << std::this_thread::get_id() << "][✉️][JOB] === response begin:\n";
          std::cout << response.as_json() << std::endl;
          std::clog << "[" << std::this_thread::get_id() << "][✉️][JOB] === response end\n";
          lock.unlock();
          std::clog << "[" << std::this_thread::get_id() << "][⚒️][JOB] finished handling '" 
                    << message_type_to_string(task.message->type) << "'\n";
        }
      }
    });
    std::thread t(worker_fn);
    worker.swap(t);
  }

  state = RUNNING;
  while (RUNNING == state) {
    std::string buf;
    std::getline(std::cin, buf);
    if (buf.empty()) {
      state = SHUTDOWN;
      break;
    }
    std::clog << "[ℹ️][MSG] received: '" << buf << "'\n";
    dispatch_message(std::move(buf));
  }

  int join_count = 0;
  while (join_count != worker_count) {
    for (auto& worker : worker_pool) {
      if (worker.joinable()) {
        worker.join();
        ++join_count;
      }
    }
  }
}

void Node::stop()
{
  state = Node::SHUTDOWN;
  queue_condition.notify_all();
}

void Node::register_handler(MessageType type, callback_fn handler)
{
  std::clog << "[ℹ️][RPC] attempting to register handler for '" << message_type_to_string(type) << "'...\n";
  if (auto found = handler_map.find(type); found != handler_map.end()) {
    std::clog << "[❌][RPC] handler for '" << message_type_to_string(type) << "' already exists.\n";
    return;
  }
  handler_map.emplace(type, handler);
  std::clog << "[✅][RPC] handler for '" << message_type_to_string(type) << "' registered.\n";
}

void Node::dispatch_message(std::string&& raw)
{
  std::optional<Message> msg = Message::parse(raw);
  if (!msg.has_value()) {
    std::clog << "[❓][MSG] failed to parse: '" << raw << "'\n";
    return;
  }
  std::clog << "[✅][MSG] parsed: '" << msg->as_json() << "'\n";

  auto found = handler_map.find(msg->type);
  if (found == handler_map.end()) {
    std::clog << "[❓][MSG] no handler for message type: '" << message_type_to_string(msg->type) << "'\n";
    // TODO: respond with unrecognized RPC error msg?
    return;
  }

  ThreadTask new_task;
  new_task.message = std::make_shared<Message>(std::move(msg.value()));
  new_task.invoke = found->second;

  // do i move this to a scope? i am deeply schizoid-paranoid about the compiler's interpretation of this
  std::unique_lock lock(mutex_thread_tasks);
  task_queue.emplace(std::move(new_task));
  lock.unlock();
  queue_condition.notify_one();
}

void Node::write_response(json response)
{
  std::unique_lock lock(mutex_write_response);
  std::cout << response << std::endl;
}

/*
void Node::write_message_threaded()
{
  while (RUNNING == state) {
    std::shared_ptr<json> resposne;
    {
      std::unique_lock lock_head(mutex_writer_head);
      std::unique_lock lock_tail(mutex_writer_tail);
      if (write_queue_tail != write_queue_head)
        lock_tail.unlock();
      condition_variable.wait(lock_head, [this]{ return nullptr != write_queue_head; });
      resposne = write_queue_head->response;
      write_queue_head = write_queue_head->next;
      if (nullptr == write_queue_head) {
      }
      lock_head.unlock();
    }
    std::cout << resposne;
  }
}
*/
