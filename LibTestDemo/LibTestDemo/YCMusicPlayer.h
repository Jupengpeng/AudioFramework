//
//  YCMusicPlayer.h
//  YCiOSPlayerLib
//
//  Created by yintao on 2016/11/14.
//  Copyright © 2016年 yintao. All rights reserved.
//


#import <Foundation/Foundation.h>
#import <YCiOSPlayerLib/YCiOSPlayerLib.h>
typedef NS_ENUM(NSUInteger, YCPlayerPlayStatus) {
    YCPlayerPlayStarting = 0,
    YCPlayerPlayPlaying,
    YCPlayerPlayPaused,
    YCPlayerPlayStopped,
    YCPlayerPlayPrepared
    
};

typedef NS_ENUM(NSUInteger, YCPlayerCircleType) {
    YCPlayerCircleTypeSingle = 0,//单曲循环
    YCPlayerCircleTypeList,//列表循环
    YCPlayerCircleTypeNormal
};

@protocol YCMusicPlayerDelegate <NSObject>


@optional
/*
 *  播放状态改变
 */

- (void)YCMusicPlayerChangingStatus:(YCPlayerPlayStatus)playStatus;

/*
 *  播放时间变化
 */
- (void)YCMusicPlayerPlayChangingPosition:(CGFloat)position;

/*
 *  当前歌曲播放结束
 */
- (void)YCMusicPlayerPlayCompletionOfFileUrl:(NSString *)fileUrl;



@end




@interface YCMusicPlayer : NSObject



+ (YCMusicPlayer *)shareInstance;

/**
 *  协议
 */
@property (nonatomic,weak) id<YCMusicPlayerDelegate> delegate;



/**
 *  歌曲播放列表
 */
@property (nonatomic,strong) NSMutableArray *musicList;


/**
 *  当前歌曲位置
 */
@property (nonatomic,assign) NSInteger playerIndex;


/**
 *  歌曲列表循环状态
 */
@property (nonatomic,assign) YCPlayerCircleType circleType ;



/**
 *  歌曲播放状态
 */
@property (nonatomic,assign) YCPlayerPlayStatus playStatus;


/**
 *  歌曲总时长(秒)
 */
@property (nonatomic,assign) NSInteger duration;

/**
 * 歌曲当前时间
 */
@property (nonatomic,assign) float position;


/**
 *  加载歌曲并播放 ()
 *  playUrl：本地地址或者网络url
 */
- (void)playWithUrl:(NSString *)playUrl;

/*
 *  设置缓存地址
 */
- (void)setCacheFilePath:(NSString *)filePath;

/**
 *
 */
- (void)start;

/**
 *  停止
 */
- (void)stop;

/**
 *  暂停
 */
- (void)pause;



/**
 *  恢复
 */
- (void)resume;

/**
 *  下一首
 */
- (void)nextPlay;

/**
 *  上一首
 */
- (void)lastPlay;

/**
 *  设置当前时间
 */


- (void)setPlayPercent:(float)percent;
/*
* 定位具体时间
*/
- (void)setPosition:(float)position;


/**
 *  获取缓存本地文件信息
 *  如果filepath = nil  获取正在加载的缓存信息
 *
 *  实例：
 *  NSFileCreationDate = "2016-11-15 06:24:00 +0000";
 *  NSFileExtensionHidden = 0;
 *  NSFileGroupOwnerAccountID = 20;
 *  NSFileGroupOwnerAccountName = staff;
 *  NSFileModificationDate = "2016-11-15 06:24:05 +0000";
 *  NSFileOwnerAccountID = 501;
 *  NSFilePosixPermissions = 420;
 *  NSFileReferenceCount = 1;
 *  NSFileSize = 2555904;
 *  NSFileSystemFileNumber = 13782746;
 *  NSFileSystemNumber = 16777220;
 *  NSFileType = NSFileTypeRegular;

 *
 */
- (NSDictionary *)getFileInfoForFilePath:(NSString *)filePath;

/**
 *  清空缓存
 */
- (void)removeLocalCache;


/**
 *
 */

@end
