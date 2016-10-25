package com.sds.android.media;

public interface MediaPlayerObserver {
	void onPrepeared();
	void onStarted();
	void onPaused();
	void onCompleted();
	void onError(int err);
	void onRecordFinished(int duration);
}