//
//  ViewController.m
//  FrameworkDemo
//
//  Created by tank on 2019/2/14.
//  Copyright © 2019 tank. All rights reserved.
//

#import "ViewController.h"
#import <StaticFramework/StaticFramework.h>
#import <DynamicFramework/DynamicFramework.h>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [[StaticFramework new] log];
    [[DynamicFramework new] log];
}


@end
