//
//  VRViewController.h
//  Vagiosity-Rover
//
//  Created by Khapugin Stepan on 26/02/14.
//  Copyright (c) 2014 Sibers. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface VRViewController : UIViewController 

@property (nonatomic, strong) IBOutletCollection(UISlider) NSArray * valueSliders;
@property (nonatomic ,strong) IBOutletCollection(UILabel)  NSArray * valueLabels;
@property (nonatomic, weak) IBOutlet UIImageView * imageView;
@end
