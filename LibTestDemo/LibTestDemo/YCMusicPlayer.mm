//
//  YCMusicPlayer.m
//  YCiOSPlayerLib
//
//  Created by yintao on 2016/11/14.
//  Copyright © 2016年 yintao. All rights reserved.
//

#import "YCMusicPlayer.h"
#import "NSString+NSMD5Addition.h"
#import <MediaPlayer/MediaPlayer.h>
@interface YCMusicPlayer ()
{
    //当前播放url
    NSString *_currentPlayUrl;
    NSString *_currentFilePath;
    BOOL _buffered;
    
    
}

@property (nonatomic,strong) CADisplayLink *displayLink;

//@property (nonatomic,strong) NSFileManager *fileManager;

//@property (nonatomic,copy) NSString *cachePath;
@end

@implementation YCMusicPlayer
static YCMusicPlayer *_player;
static TTMediaPlayerProxy *_proxy;
static NSFileManager *_fileManager;
static NSString *_cachePath;

+ (YCMusicPlayer *)shareInstance{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _player = [[YCMusicPlayer alloc] init];
        _proxy = [[TTMediaPlayerProxy alloc] init];
//        _proxy.interruptedWhilePlaying = YES;
        _fileManager = [[NSFileManager alloc] init];

        //设置缓存目录
        NSString *docPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)lastObject];
        NSString *mp3CachePath = [NSString stringWithFormat:@"%@/OnlineMp3Cache",docPath];
        if (![[NSFileManager defaultManager]  fileExistsAtPath:mp3CachePath]) {
            [_fileManager createDirectoryAtPath:mp3CachePath withIntermediateDirectories:YES attributes:nil error:nil];
        }else{
            
        }
        _cachePath = mp3CachePath;
    });
    return _player;
}

+ (instancetype)alloc{
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _player = [[super alloc] init];
        _proxy = [[TTMediaPlayerProxy alloc] init];
        _fileManager = [[NSFileManager alloc] init];
        
        //设置缓存目录
        NSString *docPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)lastObject];
        NSString *mp3CachePath = [NSString stringWithFormat:@"%@/OnlineMp3Cache",docPath];
        if (![[NSFileManager defaultManager]  fileExistsAtPath:mp3CachePath]) {
            [_fileManager createDirectoryAtPath:mp3CachePath withIntermediateDirectories:YES attributes:nil error:nil];
        }else{
            
        }
        _cachePath = mp3CachePath;
    });
    return _player;
    
}


#pragma mark - 缓存文件操作方法

- (void)removeLocalCache{
    [self removeLocalCacheWithFilePath:_cachePath];
    //重设缓存目录
    NSString *docPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)lastObject];
    NSString *mp3CachePath = [NSString stringWithFormat:@"%@/OnlineMp3Cache",docPath];
    if (![[NSFileManager defaultManager]  fileExistsAtPath:mp3CachePath]) {
        [_fileManager createDirectoryAtPath:mp3CachePath withIntermediateDirectories:YES attributes:nil error:nil];
    }else{
        
    }
}

- (void)removeLocalCacheWithFilePath:(NSString *)filePath{
    if ([_fileManager fileExistsAtPath:filePath]) {
        NSError *error = nil;
        [_fileManager  removeItemAtPath:filePath error:&error];

        if (error) {
            NSLog(@"文件删除错误%@",[error localizedDescription]);
        }else{
            NSLog(@"文件已经清空了");
        }
    }
}

//获取缓存下所有文件
- (NSArray *)getLocalCachedFiles{
   return  [_fileManager subpathsAtPath:_cachePath];
}

//获取文件信息

- (NSDictionary *)fileInfoForFilePath:(NSString *)filePath{
    NSError *error = nil;
   NSDictionary *userInfo = [_fileManager attributesOfItemAtPath:filePath error:&error];
    if (error) {
        NSLog(@"获取文件名失败 %@",[error localizedDescription]);
    }
    return userInfo;
}

- (NSDictionary *)getFileInfoForFilePath:(NSString *)filePath{
    NSError *error = nil;
    NSDictionary *fileInfo = nil;
    if (!filePath.length) {
        filePath = _currentFilePath;
    }
    fileInfo = [_fileManager attributesOfItemAtPath:filePath error:&error];
    if (error) {
        NSLog(@"获取文件名失败 %@",[error localizedDescription]);
    }
    return fileInfo;
}

- (void)setCacheFilePath:(NSString *)filePath{
    
}


#pragma mark - 播放方法


- (void)setLockScreenNowWithPlayingInfo:(NSDictionary *)playInfo{
    
    
    if (NSClassFromString(@"YCMusicPlayer")) {
        NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
        
        
        [dict setObject:playInfo[@"title"] forKey:MPMediaItemPropertyTitle];
        [dict setObject:playInfo[@"artist"] forKey:MPMediaItemPropertyArtist];
        [dict setObject:playInfo[@"albumTitle"] forKey:MPMediaItemPropertyAlbumTitle];
        
        UIImage *newImage = [UIImage imageNamed:playInfo[@"artWork"]];
        [dict setObject:[[MPMediaItemArtwork alloc] initWithImage:newImage] forKey:MPMediaItemPropertyArtwork];
        [[MPNowPlayingInfoCenter defaultCenter] setNowPlayingInfo:dict];
    }
    
}

- (void)playWithUrl:(NSString *)playUrl{

    
    //缓冲标记复位
    _buffered = NO;
    
    _currentPlayUrl = playUrl;
    [_proxy playWithUrl:playUrl];
    

    
    [self setLockScreenNowWithPlayingInfo:@{@"title":playUrl,
                                           @"artist":@"jupop",
                                           @"albumTitle":@"哼哼哈嘿",
                                            @"artWork":@"smilgaki.jpg"}];
    
    
    
    NSString *filepath  = [NSString stringWithFormat:@"%@/%@.mp3",_cachePath,[playUrl stringToMD5]];
    [_proxy SetCacheFilePath:filepath];
    
    
    
    
    _currentFilePath = filepath;
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playerNotifyEventWithMsg:) name:@"PlayerNotifyEvent"  object:nil];

    _player.playStatus = YCPlayerPlayPrepared;
    
}

/*
 *  接受播放器的通知
 */
- (void)playerNotifyEventWithMsg:(NSNotification *)notification{
    
    TTMsgObject* pMsg;
    if (notification.object) {
         pMsg=notification.object;

    }
    
    switch (pMsg.iMsg) {
            
        case ENotifyComplete:
        {
            //播放完成，结束计时器
            [_displayLink setPaused:YES];
            
            if ([self.delegate respondsToSelector:@selector(YCMusicPlayerPlayCompletionOfFileUrl:)]) {
                [self.delegate YCMusicPlayerPlayCompletionOfFileUrl:_currentPlayUrl];
            }
            
            return;
            //下面为加入播放url数组自动播放
            //一首歌结束 ，点击下一首
            [self nextPlay];
        }
            break;
        case ENotifyPrepare:
        {
            [_proxy start];
            _buffered = YES;
            NSLog(@"歌曲%@缓冲完成",_currentPlayUrl);
        }
            break;
        case ENotifyPlay:
        {
            _player.playStatus = YCPlayerPlayPlaying;
        }
            break;
        case ENotifyPause:
        {
            _player.playStatus = YCPlayerPlayPaused;
        }
            break;
        case ENotifyClose:
        {
            _player.playStatus = YCPlayerPlayStopped;

        }
            break;
        case ENotifyBufferingDone:
        {

        }
            break;
        default:
            break;
    }
    
}


- (void)start{

    [_proxy start];


}

- (void)stop{
    [_proxy stop];
}


- (void)pause{
    [_proxy pause];

}

- (void)resume{
    [_proxy resume];

}

- (void)nextPlay{

    
    switch (self.circleType) {
        case YCPlayerCircleTypeList:
        case YCPlayerCircleTypeSingle:{
            if (self.playerIndex == self.musicList.count - 1) {
                //最后一首歌曲，跳到第一首
                self.playerIndex = 0;
            }else{
                self.playerIndex +=1;
            }
            
            NSString *playUrl = [self.musicList objectAtIndex:self.playerIndex];
            [_proxy playWithUrl:playUrl];
        }
            break;
        case YCPlayerCircleTypeNormal:
        {
            
        }
            break;
        default:
            break;
    }


}

- (void)lastPlay{
    switch (self.circleType) {
        case YCPlayerCircleTypeList:
        case YCPlayerCircleTypeSingle:
        case YCPlayerCircleTypeNormal:{
            if (self.playerIndex == 0) {
                //第一首歌
                self.playerIndex=self.musicList.count-1;
            }else{
                self.playerIndex -=1;
            }
        }
            break;

        default:
            break;
    }

    NSString *playUrl = [self.musicList objectAtIndex:self.playerIndex];
    [_proxy playWithUrl:playUrl];

}

//时间读秒
- (void)displayTiming{
    
    CMTime currentTime = [_proxy getPosition];
    CGFloat position =( currentTime.value )/(float)(currentTime.timescale);

    
    if ([self.delegate respondsToSelector:@selector(YCMusicPlayerPlayChangingPosition:)]) {
        [self.delegate YCMusicPlayerPlayChangingPosition:position];
    }
}

//设置播放时间
- (void)setPlayPercent:(float)percent{
    
    if (self.duration > 0) {
        [_proxy setPosition:CMTimeMake(percent * self.duration * 100, 100)];
    }
    
}


- (void)setPosition:(float)position{
    
    if (self.duration > 0) {
        [_proxy setPosition:CMTimeMake(position * 100, 100)];
    }
}


//播放状态变化
- (void)setPlayStatus:(YCPlayerPlayStatus)playStatus{
    _playStatus = playStatus;
    
    if ([self.delegate respondsToSelector:@selector(YCMusicPlayerChangingStatus:)]) {
        [self.delegate YCMusicPlayerChangingStatus:_playStatus];
    }
    
    switch (playStatus) {
        case YCPlayerPlayPrepared:
        {
            
        }
            break;
        case YCPlayerPlayStarting:
        {
            
        }
            break;
        case YCPlayerPlayPlaying:
        {

            [self.displayLink setPaused:NO];
        }
            break;
        case YCPlayerPlayPaused:
        {
            [self.displayLink setPaused:YES];

        }
            break;
        case YCPlayerPlayStopped:
        {
            [self.displayLink setPaused:YES];

        }
            break;

        default:
            break;
    }
    
}


#pragma mark - lazy load

- (CADisplayLink *)displayLink{
    if (!_displayLink) {
        _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(displayTiming)];
        _displayLink.frameInterval = 6;
        [_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
        [self.displayLink setPaused:YES];

    }
    return _displayLink;
}



- (NSMutableArray *)musicList{
    if (!_musicList) {
        _musicList = [[NSMutableArray alloc] init];
    }

    return _musicList;
}

- (YCPlayerCircleType)circleType{
    switch (_circleType) {
        case YCPlayerCircleTypeList:
        {
            
        }
            break;
        case YCPlayerCircleTypeSingle:
        {
            if (_currentPlayUrl) {
                self.musicList = [NSMutableArray arrayWithObject:_currentPlayUrl];
            }
        }
            break;
        case YCPlayerCircleTypeNormal:
        {
            
        }
            break;
        default:
            break;
    }
    return _circleType;
}

- (NSInteger)duration{

    if (_buffered) {
        CMTime totalDuration = [_proxy duration];
        _duration = totalDuration.value/(float)totalDuration.timescale;
    }else{
        NSLog(@"歌曲还未缓冲完，文件信息尚未获取");
    }
    return _duration;
}




@end
