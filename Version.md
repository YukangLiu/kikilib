# 版本说明

旧版本都放在old_version文件夹中<br>
<br>
<br>
V0.01:<br>
1、EventMaster->EventManager->EventEpoller,Timer,EventService->SocketReader,SocketWritter->Socket层次结构<br>
2、双缓存队列线程池工具。<br>
3、双缓存队列日志工具。<br>
4、定时器工具。<br>
5、事件优先级功能。<br>
<br>
<br>
V0.02:<br>
1、取消用户实现的工厂，通过模板传入用户实现的EventService子类，通过内置工厂实现类型约束。√<br>
2、ringbuffer无锁队列日志工具。<br>
3、日志加入时间戳。<br>
4、优雅断开连接。<br>
5、减少read调用次数，虽然epoll用的是LT，但是每次读事件尽量只read一次，提高效率。<br>
7、移除了timer中的锁，将EventManager中所有组件的锁全部移到EventManager中。<br>
<br>
<br>
V0.03:<br>
1、加入对象池，减少EventService的new和delete，改这个会涉及到很多地方的改动，工作量不小。<br>
2、持续debug和调优。<br>
3、增加性能测试报告。<br>