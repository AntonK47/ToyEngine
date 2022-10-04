#include "Logger.h"

#include <g3log/logworker.hpp>
#include <g3log/loglevels.hpp>
#include <iostream>

namespace 
{
	std::unique_ptr<g3::LogWorker> worker = nullptr;

	struct ConsoleLoggerSink
	{
	private:
		static constexpr auto defaultColorSequence = "\033[37m";
		static constexpr auto errorColorSequence = "\033[31m";
		static constexpr auto warningColorSequence = "\033[33m";
		static constexpr auto infoColorSequence = "\033[32m";
	public:
		void ReceiveLogMessage(g3::LogMessageMover logEntry)
		{
			auto startSequence = defaultColorSequence;
			const auto endSequence = "\033[m";

			const auto level = logEntry.get()._level;

			if (level.value == WARNING.value)
			{
				startSequence = warningColorSequence;
			}
			if (level.value == VULKAN_VALIDATION_ERROR.value)
			{
				startSequence = errorColorSequence;
			}
			if (level.value == VALIDATION_FAILED.value)
			{
				startSequence = warningColorSequence;
			}
			if (level.value == G3LOG_DEBUG.value)
			{
				startSequence = infoColorSequence;
			}
			if (g3::internal::wasFatal(level))
			{
				startSequence = errorColorSequence;
			}

			std::cout << defaultColorSequence << endSequence << startSequence << logEntry.get().message() << endSequence << defaultColorSequence << endSequence << std::endl << std::endl;
		}
	};

}

void toy::core::logger::initialize()
{
	//TODO: create the "Log" folder first.
	worker = g3::LogWorker::createLogWorker();
	auto consoleLoggerSinkHandle = worker->addSink(std::make_unique<ConsoleLoggerSink>(), &ConsoleLoggerSink::ReceiveLogMessage);
	auto FileLoggerSinkHandler = worker->addSink(std::make_unique<g3::FileSink>("Log", "./Logs/"), &g3::FileSink::fileWrite);

#ifdef G3_DYNAMIC_LOGGING

        g3::only_change_at_initialization::addLogLevel(VULKAN_VALIDATION_ERROR, true);
#endif
	g3::initializeLogging(worker.get());
}

void toy::core::logger::deinitialize()
{
	worker.release();
	worker = nullptr;
}
