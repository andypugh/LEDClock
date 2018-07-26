/*
 * SyncControl.cpp
 *
 *  Created on: 10.03.2012
 *      Author: test123
 */

#include "SyncControl.h"

static SyncControl* _this;

static void doStop()
{
	_this->_stopAlarmID = -1;
	_this->stopSync();
}

static void doStart()
{
	_this->startSync();
}

static void syncTimeInternal(time_t time)
{
	_this->syncTime(time);
}

void SyncControl::startSync()
{
	Serial.println("start");
	_radioclock->start();
	if (_syncState == initSync || _syncState == firstSyncSleep)
	{
		_syncState = firstSync;
		if (_firstSyncDuration > 0)
			// stop 2 secs later, so we have to to sync
			_stopAlarmID = Alarm.timerOnce((time_t)(_firstSyncDuration*60 + 2), doStop);
	}
	else if (_syncState == nextSyncSleep)
	{
		_syncState = nextSync;
	}

}

void SyncControl::stopSync()
{
	Serial.println("stop");
	_radioclock->stop();
	if (_stopAlarmID != -1)
	{
		Alarm.disable(_stopAlarmID);
		_stopAlarmID = -1;
	}
	if (_syncState == firstSync)
	{
		_syncState = firstSyncSleep;
		if (_firstSyncCycle > 0)
			// 10 secs before the minute so that the module has time to come up, otherwise we lose a minute
			Alarm.timerOnce((time_t)(_firstSyncCycle*60-10), doStart);

	}
	else if (_syncState == nextSync)
	{
		_syncState = nextSyncSleep;
		if (_syncPeriod > 0)
			// 10 secs before the minute so that the module has time to come up, otherwise we lose a minute
			Alarm.timerOnce((time_t)(_syncPeriod*60-10), doStart);
		else if (_syncHour > 0)
			// 10 secs before the minute so that the module has time to come up, otherwise we lose a minute
			Alarm.alarmOnce(_syncHour, 0, 50, doStart);
	}

}

void SyncControl::syncTime(time_t time)
{
	_callback(time);
	_syncState = nextSync;
	stopSync();
}


void SyncControl::diags()
{
	static syncStatus_t oldState = undefSync;
	if (oldState != _syncState)
	{
		Serial.print("SyncControl State: ");
		Serial.println(syncStateStr[_syncState]);
		oldState = _syncState;
	}
}

void SyncControl::start()
{
	if (_syncState == initSync)
		startSync();
}

SyncControl::SyncControl(RadioClock* radioclock, setTime_t callback, int syncPeriod, time_t syncHour, int firstSyncCycle, int firstSyncDuration) {
	_radioclock = radioclock;
	_syncPeriod = syncPeriod;
    _syncHour = syncHour;
    _firstSyncCycle = firstSyncCycle;
    _firstSyncDuration = firstSyncDuration;
    _callback = callback;
    _this = this;
    _radioclock->setTimeCallback(syncTimeInternal);
    _syncState = initSync;
    _stopAlarmID = -1;


}




