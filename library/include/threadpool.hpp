#pragma once
#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <future>

class ThreadPool;

namespace threadpool
{
	// A pair of priority and function
	using PriorityFunction = std::pair<int, std::function<void()>>;

	/**
	 * @brief A simple pair key compare
	 * Used internally for the priority queue.
	 */
	struct PairKeyCompare final
	{
		constexpr bool operator()(const PriorityFunction & lhs, const PriorityFunction & rhs) const
		{
			return lhs.first < rhs.first;
		}
	};

	using TaskQueue = std::priority_queue<PriorityFunction, std::vector<PriorityFunction>, PairKeyCompare>;

    class Transaction
    {
        friend ThreadPool;
    public:
        Transaction() = default;

		template<class F, class... Args>
		auto enqueue(F && func, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>
		{
			return enqueue(0, std::bind(std::forward<F>(func), std::forward<Args>(args)...));
		}
        
        template<class F, class... Args>
        auto enqueue(int priority, F && func, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>
        {
            using return_type = typename std::result_of<F(Args...)>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(func), std::forward<Args>(args)...)
            );

			std::future<return_type> res = task->get_future();
			transaction.push(std::make_pair(priority, [task]() { (*task)(); }));
            return res;
        }

		inline std::size_t size() const { return transaction.size(); }

	private:
		TaskQueue transaction;
    };
}

/**
 * @brief Thread pool for managing a set amount of threads
 * A constant amount of threads are set and started. Functions can be sent
 * to handle the data externally.
 */
class ThreadPool final
{
public:
	/**
	 * @brief ThreadPool constructor
	 * @param size The size of the pool
	 * Spawns a set amount of threads. Default is amount of available cores.
	 */
	ThreadPool(std::size_t size = std::thread::hardware_concurrency());
	~ThreadPool();

	/**
	 * @brief Enqueues a function and its arguments
	 * @tparam F The function type
	 * @tparam Args Type of arguments attached to F
	 * @param func The function to call in a thread
	 * @param args The arguments being sent along to func
	 * The function enqueued will have default 0 priority.
	 */
	template<class F, class... Args>
	auto enqueue(F && func, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>
	{
		return enqueue(0, std::bind(std::forward<F>(func), std::forward<Args>(args)...));
	}

	/**
	 * @brief Enqueues a function and its arguments with a priority
	 * @tparam F The function type
	 * @tparam Args Type of arguments attached to F
	 * @param priority The priority of the function
	 * @param func The function to call in a thread
	 * @param args The arguments being sent along to func
	 * The priority will put it further up the queue if higher than previous.
	 */
	template<class F, class... Args>
	auto enqueue(int priority, F && func, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(func), std::forward<Args>(args)...)
		);

		std::future<return_type> res = task->get_future();
		{
			std::lock_guard<std::mutex> guard(task_mutex);
			tasks.push(std::make_pair(priority, [task]() { (*task)(); }));
		}
		task_cond.notify_one();
		return res;
	}

	/**
	 * @brief Helper method to create a new transaction
	 * @return A transaction which will store tasks
	 */
	threadpool::Transaction begin_transaction();

	/**
	 * @brief Commits a transaction into the pool
	 * @param trans The transaction to commit
	 */
	void commit(threadpool::Transaction & trans);

	/**
	 * @brief Check if the pool is idle
	 * @return True if idle, false otherwise
	 * An idle pool is when there is nothing in the queue and work is
	 * not currently processed.
	 */
	bool idle();

	/**
	 * @brief Wait for the pool to finish
	 * A pool is done when idle.
	 */
	void wait();

	/**
	 * @brief Get the size of the pool
	 * @return The size of the pool
	 * The size is the amount of workers in the pool.
	 */
	inline std::size_t size() const
	{
		return workers.size();
	}

private:
	// All the workers
	std::vector<std::thread> workers;

	// All the tasks
	threadpool::TaskQueue tasks;

	// Control pool flow
	std::mutex task_mutex;
	std::condition_variable task_cond;
	std::condition_variable idle_cond;
	bool finish;
	std::size_t num_workers;

	/**
	 * @brief Unsafe way to check if a pool is idle
	 * @return True if idle, false otherwise
	 */
	bool idleUnsafe();
};

#endif // THREADPOOL_HPP
