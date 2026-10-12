// Match timer: started when the tirette is pulled, ends after MATCH_DURATION seconds.
// Every blocking function of the firmware checks matchHasEnded(), so strategies never have to.

#pragma once

void matchStart();
bool matchHasStarted();
bool matchHasEnded();
unsigned long matchElapsedMs();

// Use this instead of delay() in strategies.
// Returns false if the match ended during the wait.
bool matchWait(unsigned long durationMs);
