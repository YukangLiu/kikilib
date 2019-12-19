//@Author Liu Yukang 
#include "StaticHttpService.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
//#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>

static std::string rootPath(".");

void StaticHttpService::handleReadEvent()
{
	struct stat st;

	//读http 请求的第一行数据（request line），把请求方法存进 method 中
	std::string line = readLineEndOfRN();
	int lineN = line.size();
	if (lineN <= 0)
	{//客户端还没发完
		return;
	}
	int methodEnd = 0;
	for (; methodEnd < lineN; ++methodEnd)
	{
		if (isspace(static_cast<int>(line[methodEnd])))
		{
			break;
		}
	}
	std::string method(line.begin(), line.begin() + methodEnd);

	//如果请求的方法不是 GET 的话就直接发送 response 告诉客户端没实现该方法
	if (method != "GET")
	{
		//忽略剩下的内容
		readAll();
		sendUnImpletement();
		return;
	}

	//跳过所有的空白字符(空格)
	int urlStart = methodEnd + 1, urlEnd = urlStart;
	for (; urlStart < lineN ; ++urlStart)
	{
		if (!isspace(static_cast<int>(line[urlStart])))
		{
			break;
		}
	}

	//获取url
	for (urlEnd = urlStart; urlEnd < lineN; ++urlEnd)
	{
		if (isspace(static_cast<int>(line[urlEnd])))
		{
			break;
		}
	}

	//将url拼接在htdocs后面
	std::string path =rootPath + std::string(line.begin() + urlStart, line.begin() + urlEnd);

	//如果 path 数组中的这个字符串的最后一个字符是以字符 / 结尾的话，就拼接上一个"index.html"的字符串。首页的意思
	if (path.back() == '/')
	{
		path += "index.html";
	}
	//忽略剩下的内容
	readAll();
	//在系统上去查询该文件是否存在
	if (stat(path.c_str(), &st) == -1)
	{//如果不存在，返回一个找不到文件的 response 给客户端
		sendNotFount();
	}
	else
	{//文件存在，那去跟常量S_IFMT相与，相与之后的值可以用来判断该文件是什么类型的
		if ((st.st_mode & S_IFMT) == S_IFDIR)
		{//如果这个文件是个目录，那就需要再在 path 后面拼接一个"/index.html"的字符串
			path += "/index.html";
		}

		//if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
		//{//如果这个文件是一个可执行文件，返回
		  //  int a = 0;
		//}
		//else
		{
			sendFile(path);
		}
	}
	forceClose();
}

void StaticHttpService::handleErrEvent()
{
	forceClose();
}

void StaticHttpService::sendUnImpletement()
{
	if (!sendContent("HTTP/1.0 501 Method Not Implemented\r\n")) return;
	if (!sendContent("Server: kikihttp/0.1.0\r\n")) return;
	if (!sendContent("Content-Type: text/html\r\n")) return;
	if (!sendContent("\r\n")) return;
	if (!sendContent("<HTML><TITLE>Method Not Implemented\r\n")) return;
	if (!sendContent("</TITLE></HEAD>\r\n")) return;
	if (!sendContent("<BODY><P>HTTP request method not supported.\r\n")) return;
	if (!sendContent("</BODY></HTML>\r\n")) return;
}

void StaticHttpService::sendFile(std::string& path)
{
	FILE* fp = NULL;

	//打开这个传进来的这个路径所指的文件
	fp = fopen(path.c_str(), "r");
	if (fp == NULL)
	{
		sendNotFount();
	}
	else
	{
		//打开成功后，将这个文件的基本信息封装成 response 的头部(header)并返回
		sendHeader(path);
		//接着把这个文件的内容读出来作为 response 的 body 发送到客户端
		sendBody(fp);

		fclose(fp);
	}
	
}

void StaticHttpService::sendNotFount()
{
	if(!sendContent("HTTP/1.0 404 NOT FOUND\r\n")) return;
	if(!sendContent("Server: kikihttp/0.1.0\r\n")) return;
	if(!sendContent("Content-Type: text/html\r\n")) return;
	if(!sendContent("\r\n")) return;
	if(!sendContent("<HTML><TITLE>Not Found</TITLE>\r\n")) return;
	if(!sendContent("<BODY><P>The server could not fulfill\r\n")) return;
	if(!sendContent("your request because the resource specified\r\n")) return;
	if(!sendContent("is unavailable or nonexistent.\r\n")) return;
	if(!sendContent("</BODY></HTML>\r\n")) return;
}

void StaticHttpService::sendHeader(std::string& path)
{
	//path后缀可以得到文件类型，太多了，以后有空再写
	if(!sendContent("HTTP/1.0 200 OK\r\n")) return;
	if(!sendContent("Server: kikihttp/0.1.0\r\n")) return;
	if(!sendContent("Content-Type: text/html\r\n")) return;
	if(!sendContent("\r\n")) return;
}

void StaticHttpService::sendBody(FILE* fp)
{
	char buf[1024];
	char* ret;
	//从文件文件描述符中读取指定内容
	ret = fgets(buf, sizeof(buf), fp);
	while (!feof(fp))
	{
		if (!sendContent(std::string(buf))) return;
		ret = fgets(buf, sizeof(buf), fp);
	}
}

