本文使用的 Xcode 版本是 Version 10.1 (10B61)。

开始前，为了表示严谨性，声明一下几点：

- 1. 本文描述的 库都是iOS 平台下的
- 2. **静态库**：特指 `framework 形式的静态库`，不含 .a，不是广义的静态库
- 3. **动态库**：特指 `Embedded Framework`，（除了特殊说明外，不含系统真正的动态库），也不是广义的动态库

本文示例 [GitHub地址](https://github.com/tankxie/iOSFrameworkDemo)。
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

在 iOS 8 之前，iOS 平台不支持使用动态 Framework，开发者可以使用的 Framework 只有苹果自家的 UIKit.Framework，Foundation.Framework 等。这种限制可能是出于安全的考虑（[见这里的讨论](https://stackoverflow.com/questions/4733847/can-you-build-dynamic-libraries-for-ios-and-load-them-at-runtime))。换一个角度讲，因为 iOS 应用都是运行在沙盒当中，不同的程序之间不能共享代码，同时动态下载代码又是被苹果明令禁止的，没办法发挥出动态库的优势，实际上动态库也就没有存在的必要了。

由于上面提到的限制，开发者想要在 iOS 平台共享代码，唯一的选择就是打包成静态库 .a 文件，同时附上头文件（例如微信的SDK）。但是这样的打包方式不够方便，使用时也比较麻烦，大家还是希望共享代码都能能像 Framework 一样，直接扔到工程里就可以用。于是人们想出了各种奇技淫巧去让 Xcode Build 出 iOS 可以使用的 Framework，具体做法参考这里和这里，这种方法产生的 Framework 还有 “伪”(Fake) Framework 和 “真”(Real) Framework 的区别。

--**分割线**--

* * *


iOS 8/Xcode 6 推出之后，iOS 平台添加了动态库的支持，同时 Xcode 6 也原生自带了 Framework 支持（动态和静态都可以），上面提到的的奇技淫巧也就没有必要了（新的做法参考[这里](http://www.cocoachina.com/ios/20141126/10322.html)）。

为什么 iOS 8 要添加动态库的支持？唯一的理由大概就是 Extension 的出现。Extension 和 App 是两个分开的可执行文件，同时需要共享代码，这种情况下动态库的支持就是必不可少的了。但是这种动态 Framework 和系统的 UIKit.Framework 还是有很大区别。系统的 Framework 不需要拷贝到目标程序中，我们自己做出来的 Framework 哪怕是动态的，最后也还是要拷贝到 App 中（App 和 Extension 的 Bundle 是共享的），因此苹果又把这种 Framework 称为 `Embedded Framework`。


### 1.3 iOS 系统下动态库的作用

从上一节的内容我们可以总结出，动态库有两大作用：

- 应用插件化，即我们可以在运行时随意替换
- 共享可执行文件，多个程序共享一份

但是，在 iOS 系统的`签名机制`和`沙盒机制`限制下，我们自制的 framework ，并不能实现插件化和不同APP间共享。

凉凉？

那么，动态库的意义何在？

一切从 `ERROR ITMS-90122` 说起。

### 1.4 说明

至此，我们对 iOS 平台下的静态库、动态库 和 framework 有了一个基本的定义。

我们需要对后文中使用的一些概念进行范围界定:

- 后文讨论都是基于 iOS 系统
- 静态库：特指 framework 形式的静态库，不含 .a 
- 动态库：特指 `Embedded Framework`，除了特殊说明外，不含系统真正的动态库

## 2. Xcode 中手动创建静态和动态的 framework

Xcode6之后，苹果在iOS上开放了动态库，下图是当前版本 Xcode 提供的创建动态framework和静态library的模板。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-3fa4180e62f2f7dc.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

由于本节研究的是framework，所以采用第一个模板。

通过设置 **Build Setting --> Linking --> Mach-O Type** 为 `Static`，打包出来的framework就是静态的。这个选项默认是 `Dynamic`。


本节将使用这两个模板，分别创建一个动态的framework--`DynamicFramework.framework`，一个静态的framework -- `StaticFramework.framework`。

### 2.1 制作动态framework

动态库的 mach-o type 使用默认的 Dynamic。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-b4a4436d2c0c60ff.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

为了方便测试，给framework添加一个同名的类，并给这个类添加一个`log`方法。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-0c9f491388a478d1.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

实现如下：

```objc
@implementation DynamicFramework
    
- (void)log{
    NSLog(@"Hello Dynamic Framework!");
}

@end
```

### 2.2 创建静态 framework

静态库的 mach-o type 使用修改为的 Static。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-b5791166bf69636d.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

同样的，给framework添加一个同名的类，并给这个类添加一个`log`方法。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-a98e0a133666f842.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


实现如下：

```objc
@implementation StaticFramework
    
- (void)log{
    NSLog(@"Hello Static Framework!");
}
    
@end
```

## 3. 使用

新建一个 Single View App ，在 APP里面测试我们上面创建的 动态framework 和静态 framework。

这个项目命名为 FrameworkDemo。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-3f715c5f03d6e405.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

为了测试方便，我们在模拟器 iPhone XR 中测试。

### 3.1 静态库的使用

StaticFramework 项目中，我们 target 选择模拟器 iPhone XR，然后编译，编译成功之后，在 Product 文件夹下面会出现一个 StaticFramework.framework。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-fbc5b70e8d2c3290.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

我们将这个 StaticFramework.framework 添加到 FrameworkDemo 这个项目中。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-b52de4820f74a9eb.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

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

### 3.2 动态库的使用

和 3.1 展示的过程一样，我们将 DynamicFramework.framework 添加到 demo 工程中，并且调用。

```objc
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

```

运行 demo，程序crash了。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-cb78652c64ec30cb.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

动态库加载失败了。

```
dyld: Library not loaded: @rpath/DynamicFramework.framework/DynamicFramework
```

#### "dyld: Library not loaded"解决方案

需要在工程的 `General` 里的 `Embedded Binaries` 添加这个动态库才能使用。 

![image.png](https://upload-images.jianshu.io/upload_images/1444901-1d351111871148c7.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

将 DynamicFramework.framework 添加进去。

![image.png](https://upload-images.jianshu.io/upload_images/1444901-cd48af0fb9dc841d.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


重新运行代码，程序正常运行。

```
2019-02-14 19:32:23.819682+0800 FrameworkDemo[39982:1415499] Hello Static Framework!
2019-02-14 19:32:23.819802+0800 FrameworkDemo[39982:1415499] Hello Dynamic Framework!
```

### 3.3 二者差异

#### Xcode 配置

前面已经看出来了，是用动态库的时候，需要额外的在 Xcode 中进行动态库配置。

#### ipa 包中表现

二者在 ipa 包中表现也不一致。

我们打开 FramewoDemo 的 ipa 包发现：

- 动态库单独放在一个文件夹中
- 静态库和源代码一起，打成一个二进制文件

![image.png](https://upload-images.jianshu.io/upload_images/1444901-4e24b93a027232cf.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

## 4. 总结

本文介绍了 iOS framework 相关的基础知识，弄清楚了动态库和静态库的构建和使用，但事实上，我们实际开发中，我们并不是通过这种原始的方式来构建 framework，现在比较通用的方案是基于cocopods来做的。

下文，将介绍 cocopods 和 framework 的关系。

## 5. 参考文章

[iOS 静态库，动态库与 Framework](https://skyline75489.github.io/post/2015-8-14_ios_static_dynamic_framework_learning.html)
[iOS动态库、静态库及使用场景、方式
](http://weslyxl.coding.me/2018/03/15/2018/3/iOS%E5%8A%A8%E6%80%81%E5%BA%93%E3%80%81%E9%9D%99%E6%80%81%E5%BA%93%E5%8F%8A%E4%BD%BF%E7%94%A8%E5%9C%BA%E6%99%AF%E3%80%81%E6%96%B9%E5%BC%8F/)