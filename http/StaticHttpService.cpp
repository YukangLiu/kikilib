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

void StaticHttpService::HandleReadEvent()
{
	char buf[1024];
	struct stat st;

	//读http 请求的第一行数据（request line），把请求方法存进 method 中
	std::string line = ReadLineEndOfRN();
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
		ReadAll();
		SendUnImpletement();
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
	ReadAll();
	//在系统上去查询该文件是否存在
	if (stat(path.c_str(), &st) == -1)
	{//如果不存在，返回一个找不到文件的 response 给客户端
		SendNotFount();
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
			SendFile(path);
		}
	}
	Close();
}

void StaticHttpService::HandleErrEvent()
{
	Close();
}

void StaticHttpService::SendUnImpletement()
{
	WriteBuf("HTTP/1.0 501 Method Not Implemented\r\n");
	WriteBuf("Server: kikihttp/0.1.0\r\n");
	WriteBuf("Content-Type: text/html\r\n");
	WriteBuf("\r\n");
	WriteBuf("<HTML><TITLE>Method Not Implemented\r\n");
	WriteBuf("</TITLE></HEAD>\r\n");
	WriteBuf("<BODY><P>HTTP request method not supported.\r\n");
	WriteBuf("</BODY></HTML>\r\n");
}

void StaticHttpService::SendFile(std::string& path)
{
	FILE* fp = NULL;

	//打开这个传进来的这个路径所指的文件
	fp = fopen(path.c_str(), "r");
	if (fp == NULL)
	{
		SendNotFount();
	}
	else
	{
		//打开成功后，将这个文件的基本信息封装成 response 的头部(header)并返回
		SendHeader(path);
		//接着把这个文件的内容读出来作为 response 的 body 发送到客户端
		SendBody(fp);
	}

	fclose(fp);
}

void StaticHttpService::SendNotFount()
{
	WriteBuf("HTTP/1.0 404 NOT FOUND\r\n");
	WriteBuf("Server: kikihttp/0.1.0\r\n");
	WriteBuf("Content-Type: text/html\r\n");
	WriteBuf("\r\n");
	WriteBuf("<HTML><TITLE>Not Found</TITLE>\r\n");
	WriteBuf("<BODY><P>The server could not fulfill\r\n");
	WriteBuf("your request because the resource specified\r\n");
	WriteBuf("is unavailable or nonexistent.\r\n");
	WriteBuf("</BODY></HTML>\r\n");
}

void StaticHttpService::SendHeader(std::string& path)
{
	//path后缀可以得到文件类型，太多了，以后有空再写
	WriteBuf("HTTP/1.0 200 OK\r\n");
	WriteBuf("Server: kikihttp/0.1.0\r\n");
	WriteBuf("Content-Type: text/html\r\n");
	WriteBuf("\r\n");
}

void StaticHttpService::SendBody(FILE* fp)
{
	char buf[1024];

	//从文件文件描述符中读取指定内容
	fgets(buf, sizeof(buf), fp);
	while (!feof(fp))
	{
		WriteBuf(std::string(buf));
		fgets(buf, sizeof(buf), fp);
	}
}

