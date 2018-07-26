#ifndef _SyncControl_h
#define _SyncControl_h

#include "RadioClock.h"
#include <TimeAlarms.h>


typedef enum {undefSync, initSync, firstSync, firstSyncSleep, nextSyncSleep, nextSync
}  syncStatus_t ;

static const char * syncStateStr[] = {"UNDEFINED", "INIT", "FIRST_SYNC", "FIRST_SYNC_SLEEP",
        "NEXT_SYNC_SLEEP", "NEXT_SYNC"};

class SyncControl {
  // user-accessible "public" interface
public:
	SyncControl(RadioClock* radioclock, setTime_t callback, int syncPeriod = 24*60, time_t syncHour = -1, int startSyncCycle = 60, int startSyncDuration = 10);
	void start();
	void diags();

    // !! for internal use only. should not be invoked by user application !!
	void startSync();
	void stopSync();
	void syncTime(time_t time);
    volatile AlarmID_t _stopAlarmID;

private:
	RadioClock* _radioclock;
	int _syncPeriod;
    time_t _syncHour;
    int _firstSyncCycle;
    int _firstSyncDuration;
    setTime_t _callback;
    volatile syncStatus_t _syncState;
};

#endif
