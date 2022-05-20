#pragma once

#include <atomic>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "executor.hpp"
#include "task.hpp"

namespace async {
class scheduler;

template <concepts::executor executor_type>
class task_container {
 public:
  using task_position = std::list<std::size_t>::iterator;

  struct options {
    std::size_t reserve_size{8};
    double growth_factor{2};
  };

  task_container(std::shared_ptr<executor_type> e,
                 const options opts = options{.reserve_size = 8,
                                              .growth_factor = 2})
      : growth_factor_(opts.growth_factor),
        executor_(std::move(e)),
        executor_ptr_(executor_.get()) {
    if (executor_ == nullptr) {
      throw std::runtime_error{"task_container cannot have a nullptr executor"};
    }

    init(opts.reserve_size);
  }
  task_container(const task_container&) = delete;
  task_container(task_container&&) = delete;
  auto operator=(const task_container&) -> task_container& = delete;
  auto operator=(task_container&&) -> task_container& = delete;
  ~task_container() {
    while (!empty()) {
      garbage_collect();
    }
  }

  enum class garbage_collect_t {
    yes,
    no
  };

  auto start(async::task<void>&& user_task,
             garbage_collect_t cleanup = garbage_collect_t::yes) -> void {
    size_.fetch_add(1, std::memory_order::relaxed);

    std::scoped_lock lk{mutex_};

    if (cleanup == garbage_collect_t::yes) {
      gc_internal();
    }

    if (free_pos_ == task_idx_.end()) {
      free_pos_ = grow();
    }

    auto index = *free_pos_;
    task_[index] = make_cleanup_task(std::move(user_task), free_pos_);

    std::advance(free_pos_, 1);

    task_[index].resume();
  }

  auto garbage_collect() -> std::size_t __attribute__((used)) {
    std::scoped_lock lk{mutex_};
    return gc_internal();
  }

  auto delete_task_size() const -> std::size_t {
    std::atomic_thread_fence(std::memory_order::acquire);
    return task_to_del_.size();
  }

  auto delete_tasks_empty() const -> bool {
    std::atomic_thread_fence(std::memory_order::acquire);
    return task_to_del_.empty();
  }

  auto size() const -> std::size_t {
    return size_.load(std::memory_order::relaxed);
  }

  auto empty() const -> bool { return size() == 0; }

  auto capacity() const -> std::size_t {
    std::atomic_thread_fence(std::memory_order::acquire);
    return task_.size();
  }

  auto garbage_collect_and_yield_until_empty() -> async::task<void> {
    while (!empty()) {
      garbage_collect();
      co_await executor_ptr_->yield();
    }
  }

  task_container(executor_type& e,
                 const options opts = options{.reserve_size = 8,
                                              .growth_factor = 2})
      : growth_factor_(opts.growth_factor), executor_ptr_(&e) {
    init(opts.reserve_size);
  }

 private:
  auto grow() -> task_position {
    auto last_pos = std::prev(task_idx_.end());
    std::size_t new_size = task_.size() * growth_factor_;
    for (std::size_t i = task_.size(); i < new_size; ++i) {
      task_idx_.emplace_back(i);
    }
    task_.resize(new_size);

    return std::next(last_pos);
  }

  auto gc_internal() -> std::size_t {
    std::size_t deleted{0};
    if (!task_to_del_.empty()) {
      for (const auto& pos : task_to_del_) {
        task_idx_.splice(task_idx_.end(), task_idx_, pos);
      }
      deleted = task_to_del_.size();
      task_to_del_.clear();
    }
    return deleted;
  }

  auto make_cleanup_task(task<void> user_task, task_position pos)
      -> async::task<void> {
    co_await executor_ptr_->schedule();

    try {
      co_await user_task;
    } catch (const std::exception& e) {
      std::cerr << "coro::task_container user_task had an unhandled exception "
                   "e.what()= "
                << e.what() << "\n";
    } catch (...) {
      std::cerr << "coro::task_container user_task had unhandle exception, not "
                   "derived from std::exception.\n";
    }

    std::scoped_lock lk{mutex_};
    task_to_del_.push_back(pos);

    size_.fetch_sub(1, std::memory_order::relaxed);
    co_return;
  }

  std::mutex mutex_{};

  std::atomic<std::size_t> size_{};
  std::vector<task<void>> task_;
  std::list<std::size_t> task_idx_{};

  std::vector<task_position> task_to_del_{};

  task_position free_pos_{};

  double growth_factor_{};

  std::shared_ptr<executor_type> executor_{nullptr};

  executor_type* executor_ptr_{nullptr};

  friend scheduler;

  auto init(std::size_t reserve_size) -> void {
    task_.resize(reserve_size);
    for (std::size_t i = 0; i < reserve_size; ++i) {
      task_idx_.emplace_back(i);
    }
    free_pos_ = task_idx_.begin();
  }
};

}  // namespace async
