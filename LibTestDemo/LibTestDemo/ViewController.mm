//
//  ViewController.m
//  LibTestDemo
//
//  Created by yintao on 2016/11/13.
//  Copyright © 2016年 yintao. All rights reserved.
//

#import "ViewController.h"

#import "YCMusicPlayer.h"
#import "NSString+NSMD5Addition.h"
@interface ViewController ()<YCMusicPlayerDelegate>
{
    YCMusicPlayer *_player;
    BOOL _isSliding;
}


@property (weak, nonatomic) IBOutlet UILabel *curentTImeLabel;
@property (weak, nonatomic) IBOutlet UILabel *durationLabel;
@property (weak, nonatomic) IBOutlet UILabel *playState;
@property (weak, nonatomic) IBOutlet UILabel *fileInfoLabel;
@property (weak, nonatomic) IBOutlet UISlider *positionSlider;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    [_positionSlider addTarget:self action:@selector(positionDragOut:) forControlEvents:UIControlEventTouchUpInside];
//    _positionSlider.continuous = NO;

    _player = [YCMusicPlayer shareInstance];
    _player.delegate = self;

    
}
- (IBAction)playNow:(id)sender {
    
//    NSString *filepath  = @"/Users/yintao/Desktop/Cache/cache.mp3";
//    [_proxy SetCacheFilePath:filepath];
    
    NSString *path1 =  @"http://audio.yinchao.cn/accompaniment/2016071500000000001.mp3";
    ;
    
    //    @"http://audio.yinchao.cn/accompaniment/2016071500000000001.mp3";
    NSString *path2 = [[NSBundle mainBundle] pathForResource:@"Omi,AronChupa-Drop In the Ocean" ofType:@"mp3"];
    _player.musicList = [NSMutableArray arrayWithArray: @[path1,path2]];
    _player.circleType = YCPlayerCircleTypeList;
    _player.playerIndex = 0;
    [_player playWithUrl:path1];
    
    
}


- (IBAction)stopNow:(id)sender {
    
    [_player stop];
}

- (IBAction)resumePlay:(id)sender {
    [_player resume];
    
}
- (IBAction)pausePlay:(UIButton *)sender {
    [_player pause];
}

- (IBAction)positionChanged:(UISlider *)sender {
    
    NSLog(@"正在改变");
    _isSliding = YES;

}


- (IBAction)positionDragOut:(UISlider *)sender {
    NSLog(@"位置已经好了");
    [_player setPlayPercent:sender.value];
    _isSliding = NO;
}

- (IBAction)getFileInfo:(id)sender {
    
    NSLog(@"缓存文件信息%@",[_player getFileInfoForFilePath:nil]);
    
}


- (IBAction)clearUp:(id)sender {
    [_player removeLocalCache];
    
    
}

#pragma mark - YCMusicPlayerDelegate

- (void)YCMusicPlayerPlayChangingPosition:(CGFloat)position{
    
    if (_isSliding) {
        return;
    }
    NSLog(@"%@", [NSString stringWithFormat:@"当前时间 ：%.2f",position]);
    self.curentTImeLabel.text = [NSString stringWithFormat:@"当前时间 ：%.2f",position];
    
    _positionSlider.value = position/_player.duration;
}

- (void)YCMusicPlayerChangingStatus:(YCPlayerPlayStatus)playStatus{
    NSLog(@"播放状态 %lu",(unsigned long)playStatus);
    
    NSString *stateStr = @"";
    switch (playStatus) {
        case YCPlayerPlayPrepared:
        {
            stateStr = @"准备";
        }
            break;
        case YCPlayerPlayStarting:
        {
            stateStr = @"开始";

        }
            break;
        case YCPlayerPlayPlaying:
        {
            stateStr = @"正在播放";
        }
            break;
        case YCPlayerPlayPaused:
        {
            stateStr = @"暂停";

        }
            break;
        case YCPlayerPlayStopped:
        {
            stateStr = @"停止";

        }
            break;
        default:
            break;
    }
    _playState.text = [NSString stringWithFormat:@"%@",stateStr];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
