#pragma once

#include <thread>
#include <vector>
#include <Core.h>
#include <functional>
#include <span>

namespace toy::core
{
	constexpr auto cacheSize = std::hardware_constructive_interference_size;
	

	struct Task
	{
		std::function<void(void)> taskFunction;
		DebugLabel label = {};
		
		bool tryAcquire()
		{
			return !isAcquired_.exchange(true);
		}

		bool isAcquired()
		{
			return isAcquired_.load();
		}

		void release()
		{
			isDone_.store(true);
			isAcquired_.store(false);
		}

		bool isDone()
		{
			return isDone_.load();
		}

		void reset()
		{
			isAcquired_.store(false);
			isDone_.store(false);
		}

	private:
		alignas(cacheSize) std::atomic_bool isAcquired_ = false;
		alignas(cacheSize) std::atomic_bool isDone_ = false;
	};

	enum WorkerTag : u8
	{
		common = 0x1,
		io = 0x2,
		background = 0x4,
		rhi = 0x8
	};

	struct Barrier
	{
		Task* tasks[256];
		u32 beginIndex_{ 0 };
		u32 endIndex_{ 0 };
		WorkerTag tag{ WorkerTag::common };
		void wait();
	};

	struct TaskSystemDescriptor
	{

	};

	

	struct TaskSystem
	{
		void initialize(const TaskSystemDescriptor& descriptor);
		void deinitialize();

		Barrier run(const std::vector<Task*>& tasks, WorkerTag tag = WorkerTag::common);
		Barrier run(std::span<Task> tasks, WorkerTag tag = WorkerTag::common);
		std::vector<std::thread::id> workers();
	};

}

void testTaskSystem();
