#include "RestartRequest.h"
#include <atomic>

static std::atomic<bool> gRestartRequested{false};
static std::string gRestartReason;

void requestRestart(const std::string& reason)
{
	gRestartReason = reason;
	gRestartRequested.store(true);
}

bool isRestartRequested()
{
	return gRestartRequested.load();
}

std::string getRestartReason()
{
	return gRestartReason;
}
