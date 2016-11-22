//
//  ViewController.m
//  YCiOSPlayer
//
//  Created by yintao on 2016/10/31.
//  Copyright © 2016年 yintao. All rights reserved.
//

#import "ViewController.h"



#import <YCPlayer/YCPlayer.h>
@interface ViewController ()
{
    TTMediaPlayerProxy *_proxy;
    BOOL _canPlay;
    CMTime _totalDuration;
    CGFloat _totalValue;
}
@property (weak, nonatomic) IBOutlet UILabel *curentTImeLabel;
@property (weak, nonatomic) IBOutlet UILabel *durationLabel;
@property (weak, nonatomic) IBOutlet UILabel *playState;
@property (weak, nonatomic) IBOutlet UILabel *fileInfoLabel;

@property (weak, nonatomic) IBOutlet UITextField *startTimeText;

@property (weak, nonatomic) IBOutlet UITextField *endTimeText;


@property (weak, nonatomic) IBOutlet UISlider *progressSlider;
@property (nonatomic,strong) CADisplayLink *displayLink;
@property (weak, nonatomic) IBOutlet UISlider *leftAndRightVolume;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    
    
    _proxy = [[TTMediaPlayerProxy alloc] init];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playHere:) name:@"PlayerNotifyEvent"  object:nil];

    

    

    self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(actionTiming)];
    
    self.displayLink.frameInterval=6;
    
    [self.displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
    [self.displayLink setPaused:YES];
    
    
    
    NSString *docPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)lastObject];
    NSString *filepath  = [NSString stringWithFormat:@"%@/%@",docPath,@"213.mp3"];    [_proxy SetCacheFilePath:filepath];
    
    

    
    _progressSlider.continuous = NO;
    _progressSlider.value = 0;
    _leftAndRightVolume.value = 0;
    _leftAndRightVolume.continuous = NO;
//    _proxy SetActiveNetWorkType:<#(TTActiveNetWorkType)#>

    UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(resignAllKeyBoard)];
    [self.view addGestureRecognizer:tap];
    
//    int *freq = (short)8;
//    int *buffer = a;
//    
//    TTInt waveHeight = [_proxy getCurFreqAndWaveWithFreqBuffer:a andWaveBuffer:a andSamplenum:10];
//    TTInt waveHeight = [_proxy getCurFreqAndWaveWithFreqBuffer:10 andWaveBuffer: andSamplenum:10];
    
    NSLog(@"");
}

//收起键盘

- (void)resignAllKeyBoard{

    [self.endTimeText resignFirstResponder];
    [self.startTimeText resignFirstResponder];
}

- (void)playHere:(NSNotification *)obj{

    [_proxy start];
    
    
    
    _playState.text = [NSString stringWithFormat:@"播放状态 %d",[_proxy getPlayStatus]];
    

//    [self.displayLink setPaused:NO];


    
}

//重新开始
- (IBAction)startAction:(UIButton *)sender {
    
 
    NSString *path1 =@"http://audio.yinchao.cn/accompaniment/2016071500000000001.mp3";
//    @"http://audio.yinchao.cn/accompaniment/2016071500000000001.mp3";
   NSString *path2 = [[NSBundle mainBundle] pathForResource:@"Omi,AronChupa-Drop In the Ocean" ofType:@"mp3"];
    
    [_proxy playWithUrl:path1];
    
    [self.displayLink setPaused:NO];

    _playState.text = [NSString stringWithFormat:@"播放状态 %d",[_proxy getPlayStatus]];


}

//结束
- (IBAction)stopAction:(id)sender {
    
    [_proxy stop];
    [self.displayLink setPaused:YES];

    
    _playState.text = [NSString stringWithFormat:@"播放状态 %d",[_proxy getPlayStatus]];

}

- (IBAction)playAction:(id)sender {

    [_proxy resume];
    [self.displayLink setPaused:NO];
    
    _playState.text = [NSString stringWithFormat:@"播放状态 %d",[_proxy getPlayStatus]];


}
- (IBAction)pauseAction:(id)sender {
    [self.displayLink setPaused:YES];

    [_proxy pause];
    
    _playState.text = [NSString stringWithFormat:@"播放状态 %d",[_proxy getPlayStatus]];


}




- (IBAction)playFromThisTwo:(UIButton *)sender {
    [_startTimeText resignFirstResponder];
    [_endTimeText resignFirstResponder];
    CMTime startTime = CMTimeMake([_startTimeText.text floatValue] * 100, 100);
    CMTime endTime = CMTimeMake([_endTimeText.text floatValue] * 100, 100);

    [_proxy pause];
    
    [_proxy setPlayRangeWithStartTime:startTime EndTime:endTime];
    
    [self.displayLink setPaused:NO];



    
}

- (IBAction)progressSlider:(UISlider *)sender {

    CGFloat totalValue = _totalDuration.value/(float)_totalDuration.timescale;

    [_proxy setPosition:CMTimeMake((sender.value * totalValue) * 100, 100)];
    
}



- (void)actionTiming{

     CMTime currentTime = [_proxy getPosition];
    
    CGFloat time =( currentTime.value )/(float)(currentTime.timescale);
    
    NSLog(@"%@", [NSString stringWithFormat:@"当前时间 ：%.2f",time]);
    self.curentTImeLabel.text = [NSString stringWithFormat:@"当前时间 ：%.2f",time];
    
    

    if (!_totalValue) {
        _totalDuration = [_proxy duration];
        _totalValue = _totalDuration.value/(float)_totalDuration.timescale;
        _durationLabel.text = [NSString stringWithFormat:@"%.2f",_totalValue];
        
        _fileInfoLabel.text = [NSString stringWithFormat:@"文件大小%d——缓冲大小%d——缓冲率%d",[_proxy fileSize],[_proxy bufferedFileSize],[_proxy BufferedPercent]];
    }
}
- (IBAction)leftAndRightVolume:(UISlider *)sender {
    [_proxy setBalanceChannel:0];
}



- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
