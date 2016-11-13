//
//  ViewController.m
//  YCiOSPlayer
//
//  Created by yintao on 2016/10/31.
//  Copyright © 2016年 yintao. All rights reserved.
//

#import "ViewController.h"
#import <YCPlayer/TTAudioSink.h>
#import <YCPlayer/TTMediaPlayerWarp.h>
#import <YCPlayer/TTMediaPlayerProxy.h>
@interface ViewController ()
{
    TTMediaPlayerProxy *_proxy;
    BOOL _canPlay;
}
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    
    
    _proxy = [[TTMediaPlayerProxy alloc] init];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playHere:) name:@"PlayerNotifyEvent"  object:nil];

    
//    CTTMediaPlayerWarp * iMediaPlayer = new CTTMediaPlayerWarp((__bridge void*)proxy);
//    
//    char myChar[100];
//    TTChar* aUrl = strcpy(myChar,(char *)[path UTF8String]);
//    if (TTKErrNone == iMediaPlayer -> SetDataSourceAsync(aUrl)) {
//        
//        iMediaPlayer->Play();
//        NSLog(@"paly SetDatasourceSync");
//        
//    }
}

- (void)playHere:(NSNotification *)obj{

    [_proxy start];
//    [_proxy start];

}
- (IBAction)playAction:(id)sender {
    NSString *path = [[NSBundle mainBundle] pathForResource:@"Omi,AronChupa-Drop In the Ocean" ofType:@"mp3"];

    [_proxy playWithUrl:path];

 
}
- (IBAction)pauseAction:(id)sender {

    [_proxy stop];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
