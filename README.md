# kikilib网络库
blog:https://blog.csdn.net/weixin_42333737/article/details/103408007<br>
<br>
[版本说明](https://github.com/YukangLiu/kikilib/blob/master/Version.md)<br>
<br>
<br>
1、定位：<br>
	kikilib网络库是轻量，高性能，纯c++11，更符合OOP语言特点且易用的一个Linux服务器网络库，它没有繁琐的回调函数设置和上下文指针机制，这让它的使用和对象的生命期管理都变得更加简单。<br>
<br>
<br>
2、概述：<br>
	使用了多Reactor+非阻塞IO的并发模型，坚持One Loop Per Thread，默认根据负载情况派发新连接。<br>
	1、实现了双缓存队列的高性能线程池工具。<br>
	2、实现了RingBuffer无锁队列的高性能异步日志工具。<br>
	3、实现了定时器事件。<br>
	4、实现了事件按优先级处理。<br>
<br>
<br>
3、文件：<br>
	kikilib：网络库的源代码。<br>
	http：使用kikilib网络库实现的一个简单的静态网页服务器。<br>
	chatroom：使用kikilib网络库实现的一个简单的聊天室服务器（可以认为是广播协议）。<br>
	test：测试工程，加上http和chatroom将所有函数都使用上了。<br>
<br>
<br>
4、编译：<br>
	kikilib：运行makekikilib.sh脚本即可。<br>
	http：运行makekikihttp.sh脚本即可。<br>
	chatroom：运行makekikichatroom.sh脚本即可。<br>
<br>
<br>
5、http及chatroom使用：<br>
	http：编译后运行即可。测试站点：http://122.51.68.134/<br>
	chatroom：编译后，修改config.txt中的端口号为监听的端口号，运行即可。文件夹中提供一个client的windows下的执行文件，修改clientconfig.txt为服务器的ip和端口号运行即可。<br>
<br>
<br>
6、kikilib网络库使用：<br>
	1、继承EventService类（专属于一个socket，专注于服务该socket上发生的事件），实现其中的HandleConnectionEvent(),HandleReadEvent(),HandleErrEvent,HandleCloseEvent()函数即可，可自定义自己的私有成员，代替了大多数网络库中的context上下文指针，生命器管理也更容易。<br>
	2、将具体的EventService类型放在EventMaster模板中，服务器即可运转，每有一个新的连接到来，EventMaster就会为新的连接创建一个该事件服务对象。<br>
	另外，网络库的大部分API都在EventService类中，这让使用变得更加方便，提供以下API：<br>
	1、提供自身socket的操作API<br>
	2、提供自身事件相关的操作API<br>
	3、提供定时器相关的操作API<br>
	4、提供线程池工具的操作API<br>
	5、提供socket缓冲区的读写操作API<br>
<br>
<br>
7、并发度<br>
	测试环境：4核CPU3.70GHz，8G内存3200MHz<br>
	使用webbench对本机http进行了简单的压力测试，QPS两万多：<br>
	10000 clients, running 600 sec.<br>
	Speed=1338931 pages/min, -568964 bytes/sec.<br>
	Requests: 13389316 susceed, 0 failed.<br>
<br>
<br>
附：有什么需求或者bug，建议，问题，都可以加我微信liuyukang315讨论交流，谢谢。<br>