// Match timer: started when the tirette is pulled, ends after MATCH_DURATION milliseconds.
// Every blocking function of the firmware checks matchHasEnded(), so strategies never have to.
//
// Typical use in main:
//   pinMode(TIRETTE_PIN, INPUT_PULLUP);
//   ...wait until TIRETTE_PIN changes state (cord pulled)...
//   matchStart();
//   runStrategy();                                     // every wait/move returns early at match end
//   actuatorsStopAll();
//
// Before matchStart() the match is "not started": matchHasEnded() returns false, so the
// actuators can be homed and tested freely during the preparation time.

#pragma once

// Starts the timer (call it once, when the tirette is pulled). Logs "[MATCH] Match started".
void matchStart();

// true once matchStart() has been called.
bool matchHasStarted();

// true once MATCH_DURATION ms have elapsed since matchStart(). Always false before the start.
// Logs "[MATCH] Match time is over" the first time it returns true.
bool matchHasEnded();

// Milliseconds since matchStart(), 0 before the start.
unsigned long matchElapsedMs();

// Use this instead of delay() in strategies.
// Returns false if the match ended during the wait (it then returns early, as soon as the
// match is over), true if the full duration elapsed with the match still running.
// The steppers keep moving during the wait: FastAccelStepper generates the pulses in the
// background, not in this loop.
bool matchWait(unsigned long durationMs);
