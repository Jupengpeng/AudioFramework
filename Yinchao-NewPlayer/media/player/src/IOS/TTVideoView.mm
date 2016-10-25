//
//  TTVideoView.m
//

#import <QuartzCore/QuartzCore.h>

#import "TTVideoView.h"

@implementation TTVideoView


+(Class) layerClass
{
	return [CAEAGLLayer class];
}


- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    
    if (nil != self) {
        // Initialization code
    }
    return self;
}


- (void)drawRect:(CGRect)rect {
    // Drawing code
}


- (void)dealloc {
    [super dealloc];
}


@end
