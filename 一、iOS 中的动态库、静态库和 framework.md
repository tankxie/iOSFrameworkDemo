## 写在前面

所谓 SDK 开发，就是做一个库（library）给别人用，本文重在介绍 iOS 平台下的库 -- `framework`。

本文的结构如下：

- **基本认识**，这一部分介绍静态、静态库和framework的基本概念
- iOS 系统下的 framework 总结，这一部分介绍 iOS 平台下的库
- 在 Xcode 中手动创建各类 framework
- 使用上面创建的 framework 

本文使用的 Xcode 版本是 Version 10.1 (10B61)。

## 1. 基本认识

### 1.1 什么是库

首先来看什么是库，库(Library)说白了就是一段编译好的二进制代码，加上头文件就可以供别人使用。

什么时候我们会用到库呢？一种情况是某些代码需要给别人使用，但是我们不希望别人看到源码，就需要以库的形式进行封装，只暴露出头文件。另外一种情况是，对于某些不会进行大的改动的代码，我们想减少编译的时间，就可以把它打包成库，因为库是已经编译好的二进制了，编译的时候只需要 Link 一下，不会浪费编译时间。

上面提到库在使用的时候需要 Link，Link 的方式有两种，静态和动态，于是便产生了静态库和动态库。

我们从文件名可以直观的分辨动态库和静态库。

一般来说，动态库以 `.dylib` 或者 `.framework` 后缀结尾；静态库以 `.a` 和 `.framework` 结尾。

### 1.2 动态库、静态库和 framework 概念

> 严格意义上讲，这三个概念不在一个维度上。framework 并不是库，它只是一种打包方式，它既可以是动态库也可以是静态库。

#### 静态库

静态库即静态链接库（Windows 下的 .lib，Linux 和 Mac 下的 .a）。之所以叫做静态，是因为静态库在编译的时候会被直接拷贝一份，复制到目标程序里，这段代码在目标程序里就不会再改变了。

静态库的好处很明显，编译完成之后，库文件实际上就没有作用了。目标程序没有外部依赖，直接就可以运行。当然其缺点也很明显，就是会使用目标程序的体积增大。


#### 动态库

动态库即动态链接库（Windows 下的 .dll，Linux 下的 .so，Mac 下的 .dylib/.tbd）。与静态库相反，动态库在编译时并不会被拷贝到目标程序中，目标程序中只会存储指向动态库的引用。等到程序运行时，动态库才会被真正加载进来。

动态库的优点是，不需要拷贝到目标程序中，不会影响目标程序的体积，而且同一份库可以被多个程序使用（因为这个原因，动态库也被称作共享库）。同时，运行时才载入的特性，也可以让我们随时对库进行替换，而不需要重新编译代码。动态库带来的问题主要是，动态载入会带来一部分性能损失，使用动态库也会使得程序依赖于外部环境。如果环境缺少动态库或者库的版本不正确，就会导致程序无法运行（Linux 下喜闻乐见的 lib not found 错误）。

#### iOS Framework

除了上面提到的 .a 和 .dylib/.tbd 之外，Mac OS/iOS 平台还可以使用 Framework。

Framework 实际上是一种打包方式，将库的二进制文件，头文件和有关的资源文件打包到一起，方便管理和分发，和静态库动态库的本质是没有什么关系。

> 在 iOS 8 之前，iOS 平台不支持使用动态 Framework，开发者可以使用的 Framework 只有苹果自家的 UIKit.Framework，Foundation.Framework 等。因为 iOS 应用都是运行在沙盒当中，不同的程序之间不能共享代码，同时动态下载代码又是被苹果明令禁止的，没办法发挥出动态库的优势，实际上动态库也就没有存在的必要了。
由于上面提到的限制，开发者想要在 iOS 平台共享代码，唯一的选择就是打包成静态库 .a 文件，同时附上头文件。

iOS 8/Xcode 6 推出之后，iOS 平台添加了动态库的支持，同时 Xcode 6 也原生自带了 Framework 支持。




## 2. iOS 系统下的 framework 总结

有上面对`库`和`framework`的基本认识之后，本节对 iOS 平台下的 framework 进行阐述。

### 2.1 framework 只是一种打包方式

framework 只是一种打包方式，它只是简单的将`二进制文件`、`头文件` 以及其它的一些信息聚合在一起。

本文后面，会手动制作 static framework 和 embedded framework，这里，我们先从文件目录结构上，研究一下这两个 framework。

static framework 文件目录结构如下：

```sh
➜  StaticFramework.framework git:(master) tree
.
├── Headers
│   └── StaticFramework.h
├── Info.plist
├── Modules
│   └── module.modulemap
├── StaticFramework
└── _CodeSignature
    ├── CodeDirectory
    ├── CodeRequirements
    ├── CodeRequirements-1
    ├── CodeResources
    └── CodeSignature
```

embedded framework 文件目录结构如下：

```sh
➜  EmbeddedFramework.framework git:(master) ✗ tree
.
├── EmbeddedFramework
├── Headers
│   └── EmbeddedFramework.h
├── Info.plist
├── Modules
│   └── module.modulemap
└── _CodeSignature
    └── CodeResources

3 directories, 5 files
```

通过对比，我们发现二者并没有太大区别，所以这也佐证了，`framework只是一种打包方式，不代表库的 link 特性`。


### 2.2 iOS 系统下 framework 分类

从上面的描述可知，iOS 系统中的 framework 按照如下方式分类。

![iOS 系统中的 framework 分类](https://upload-images.jianshu.io/upload_images/1444901-001a0ada26522523.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

#### Dynamic Framework

**Dynamic Framework** ，动态库，系统提供的 framework 都是动态库，比如 `UIKit.framework`，具有所有动态库的特性。


#### Static Framework

**Static Framework**，静态库，用户可以制作，可以粗略的理解为，它等价于 `头文件 + 资源文件 + 二进制代码`，它具有静态库的属性。

#### Embedded Framework

**Embedded Framework**，这个是用户可以制作的“动态库”，它是受到 iOS 平台限制（`签名机制`和`沙盒机制`限制）的动态库，它具有`部分动态特性`，比如：

- Embedded Framework 可以在 `Extension可执行文件` 和 `APP可执行文件` 之间共享，但是不能像系统的动态库一样，在不同的 APP（进程） 中共享  
- 系统的 Framework 不需要拷贝到目标程序中，Embedded Framework 最后也还是要拷贝到 APP 中，下图所示的是，使用 Embedded Framework 的项目的 APP 目录

![使用 embedded framework 的 APP 可执行文件 目录](https://upload-images.jianshu.io/upload_images/1444901-ff6a0aa87f477e1e.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

### 2.3 Embedded Framework 存在的意义

换言之，我们在哪些场景可以利用 Embedded Framework 的特性？？

这个我们在另外一篇文章中阐明。

接下来，手动创建一个 static framework 和 embedded framework，来研究 framework 的特性。

## 3. Xcode 中手动创建 framework

下图是当前版本 Xcode 提供的创建 framework 的模板。

![framework创建模板](https://upload-images.jianshu.io/upload_images/1444901-489cc4205ab91300.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


本节将使用这两个模板，分别创建一个动态的framework--`DynamicFramework.framework`，一个静态的framework -- `StaticFramework.framework`。

### 3.1 制作 embedded framework

使用 cocoa touch framework 模板创建一个名为 EmbeddedFramework 的项目，

我们查看 **Build Setting --> Linking --> Mach-O Type** ，这个选项默认是 `Dynamic`，表示我们当前为  `embedded framework`。

![embedded framework mech-o type 设置](https://upload-images.jianshu.io/upload_images/1444901-1ab567342d9d5353.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


为了方便测试，给framework添加一个同名的类，并给这个类添加一个 `log` 实例方法。

实现如下：

```objc
@implementation EmbeddedFramework
    
- (void)log{
    NSLog(@"Hello Embedded Framework!");
}

@end
```

### 3.2 制作 static framework

使用 cocoa touch framework 模板创建一个名为 StaticFramework 的项目，通过设置 **Build Setting --> Linking --> Mach-O Type** 为 `Static Library`，将我们当前的 framework 设置为 static framework。

![static framework mech-o type 设置](https://upload-images.jianshu.io/upload_images/1444901-8db0589af359827f.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


同样的，给 framework 添加一个同名的类，并给这个类添加一个`log`实例方法，方法实现如下：

```objc
@implementation StaticFramework
    
- (void)log{
    NSLog(@"Hello Static Framework!");
}
    
@end
```

## 4. 使用

新建一个 `Single View App` ，在 APP里面测试我们上面创建的 framework，这个项目命名为 FrameworkDemo。

为了测试方便，我们只在模拟器 iPhone XR 中测试，不在其它架构平台上测试。

### 4.1 static framework 的使用

#### 生成  `StaticFramework.framework`

StaticFramework 项目中，我们 target 选择模拟器 iPhone XR，然后编译，编译成功之后，在 Product 文件夹下面会出现一个 StaticFramework.framework。

#### 使用  `StaticFramework.framework`

![](https://upload-images.jianshu.io/upload_images/1444901-b458fb0cbf4d8d16.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


我们将这个 StaticFramework.framework 添加到 FrameworkDemo 这个项目中。

![](https://upload-images.jianshu.io/upload_images/1444901-77ea107198b2a1e3.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


在 demo 工程中，使用静态framework，调用代码如下：

```objc
- (void)viewDidLoad {
    [super viewDidLoad];
    
    [[StaticFramework new] log];
}
```

控制台打印结果如下：

```
2019-02-14 19:14:31.674758+0800 FrameworkDemo[39272:1382553] Hello Static Framework!

```

调用成功。

所以静态库的使用比较简单，直接将framework 添加到项目中即可。

### 4.2 embedded framework 的使用

和 3.1 展示的过程一样，我们将生成的 EmbeddedFramework.framework 添加到 demo 工程中，并且调用。

```objc
#import "ViewController.h"
#import <StaticFramework/StaticFramework.h>
#import <EmbeddedFramework/EmbeddedFramework.h>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [[StaticFramework new] log];
    [[EmbeddedFramework new] log];
}

@end

```

运行 demo，程序crash了，控制台输出如下。

```
dyld: Library not loaded: @rpath/EmbeddedFramework.framework/EmbeddedFramework
  Referenced from: /Users/xieshoutan/Library/Developer/CoreSimulator/Devices/9124D297-42BC-467E-B343-59441AAA0FE0/data/Containers/Bundle/Application/3DAF0FC7-D040-478B-891A-AD678109380B/FrameworkDemo.app/FrameworkDemo
  Reason: image not found
```

#### "dyld: Library not loaded"解决方案

需要在工程的 `General` 里的 `Embedded Binaries` 添加这个动态库才能使用。 

![image.png](https://upload-images.jianshu.io/upload_images/1444901-f6f38d1681790c7a.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

将 DynamicFramework.framework 添加进去。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-ba06c9885a7198c1.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


重新运行代码，程序正常运行。

```
2019-02-14 19:32:23.819682+0800 FrameworkDemo[39982:1415499] Hello Static Framework!
2019-02-14 19:32:23.819802+0800 FrameworkDemo[39982:1415499] Hello Dynamic Framework!
```

### 4.3 二者使用过程中的差异比较

#### Xcode 配置

前面已经看出来了，是用动态库的时候，需要额外的在 Xcode 中进行动态库配置。

#### ipa 包中表现

二者在 ipa 包中表现也不一致。

我们打开 FramewoDemo 的 ipa 包发现：

- 动态库单独放在一个文件夹 `Frameworks` 中
- 静态库和源代码一起，打成一个二进制文件 `FrameworkDemo`

如下是 `FrameworkDemo.app` 的文件目录：

```sh
➜  FrameworkDemo.app tree
.
├── Base.lproj
│   ├── LaunchScreen.storyboardc
│   │   ├── 01J-lp-oVM-view-Ze5-6b-2t3.nib
│   │   ├── Info.plist
│   │   └── UIViewController-01J-lp-oVM.nib
│   └── Main.storyboardc
│       ├── BYZ-38-t0r-view-8bC-Xf-vdC.nib
│       ├── Info.plist
│       └── UIViewController-BYZ-38-t0r.nib
├── FrameworkDemo
├── Frameworks
│   └── EmbeddedFramework.framework
│       ├── EmbeddedFramework
│       ├── Info.plist
│       └── _CodeSignature
│           └── CodeResources
├── Info.plist
├── PkgInfo
└── _CodeSignature
    └── CodeResources

7 directories, 13 files
```

## 4. 总结

本文介绍了 iOS framework 相关的基础知识，弄清楚了 iOS 中 framework 的种类、构建方法和基本使用。

但事实上，我们实际开发中，我们并不是通过这种原始的方式来构建 framework，现在比较通用的方案是基于cocopods来做的。而且实际开发中，我们构建的 framework 远比这个复杂，包含资源文件的依赖、其它静态库的依赖等等。

后文，将介绍 cocopods 和 framework 的关系。

## 参考文档

[iOS里的动态库和静态库](https://www.zybuluo.com/qidiandasheng/note/603907)
[iOS动态库、静态库及使用场景、方式
](http://weslyxl.coding.me/2018/03/15/2018/3/iOS%E5%8A%A8%E6%80%81%E5%BA%93%E3%80%81%E9%9D%99%E6%80%81%E5%BA%93%E5%8F%8A%E4%BD%BF%E7%94%A8%E5%9C%BA%E6%99%AF%E3%80%81%E6%96%B9%E5%BC%8F/)
[本文示例 demo github 地址](https://github.com/tankxie/iOSFrameworkDemo)



