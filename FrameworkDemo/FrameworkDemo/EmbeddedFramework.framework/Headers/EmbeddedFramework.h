//
//  EmbeddedFramework.h
//  EmbeddedFramework
//
//  Created by tank on 2019/2/15.
//  Copyright © 2019 tank. All rights reserved.
//

#import <UIKit/UIKit.h>

//! Project version number for EmbeddedFramework.
FOUNDATION_EXPORT double EmbeddedFrameworkVersionNumber;

//! Project version string for EmbeddedFramework.
FOUNDATION_EXPORT const unsigned char EmbeddedFrameworkVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <EmbeddedFramework/PublicHeader.h>

@interface EmbeddedFramework : NSObject
- (void)log;
@end
