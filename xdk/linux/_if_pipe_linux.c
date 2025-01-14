﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc pipe system call document

	@module	_if_pipe.c | linux implement file

	@devnote 张文权 2021.01 - 2021.12	v6.0
***********************************************************************/

/**********************************************************************
This program is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
LICENSE.GPL3 for more details.
***********************************************************************/

#include "../xdkloc.h"

#ifdef XDK_SUPPORT_PIPE

#define PIPE_HEAD		_T("/tmp/")

res_file_t _pipe_srv(const tchar_t* pname, dword_t fmode)
{
    int flag, rt;
    char ppath[1024] = {0};
    
    if (!pname || !(*pname))
    {
        if(fmode & FILE_OPEN_WRITE)
             return STDOUT_FILENO;
         else
             return STDIN_FILENO;
    }
    
    sprintf(ppath, "%s%s", PIPE_HEAD, pname);
    
    if (0 == access(ppath, F_OK))
    {
        unlink(ppath);
    }
    
    if(0 != mkfifo(ppath, 0666))
        return INVALID_FILE;
    
    if(fmode & FILE_OPEN_WRITE)
        flag = O_WRONLY;
    else
        flag = O_RDONLY;
    
    if(fmode & FILE_OPEN_OVERLAP)
        flag |= O_NONBLOCK;
    
    rt = open(ppath, flag);
    
    return (rt < 0)? INVALID_FILE : (res_file_t)rt;
}

wait_t _pipe_listen(res_file_t pip, async_t* pb)
{
    struct timeval tv = {0};
    
    int rs;
    struct epoll_event ev = {0};
    fd_set fs = {0};

    if (pb->type == ASYNC_QUEUE)
    {
        ev.events = EPOLLIN;
        ev.data.fd = pip; 

        epoll_ctl(pb->port, EPOLL_CTL_ADD, pip, &ev);       
        rs = epoll_wait(pb->port, &ev, 1, (int)pb->timo);
        epoll_ctl(pb->port, EPOLL_CTL_DEL, pip, &ev); 

        if(rs <= 0)
        {
            return WAIT_TMO;
        }
    }
    else if (pb->type == ASYNC_EVENT)
    {
        FD_ZERO(&fs);
        FD_SET(pip, &fs);
        
        tv.tv_sec = pb->timo / 1000;
        tv.tv_usec = (pb->timo % 1000) * 1000;
        
        rs = select(pip + 1, &(fs), NULL, NULL, &tv);
        FD_CLR(pip, &fs);
        
        if(rs <= 0)
        {
            return WAIT_TMO;
        }
    }
    
    return WAIT_RET;
}

void _pipe_stop(res_file_t pip)
{
    
}

void _pipe_close(const tchar_t* pname, res_file_t pip)
{
    char ppath[1024] = {0};
    
    if(pname)
        sprintf(ppath, "%s%s", PIPE_HEAD, pname);
    
    close(pip);
    
    if(pname)
        unlink(ppath);
}

res_file_t _pipe_cli(const tchar_t* pname, dword_t fmode)
{
    int flag, rt;
    char ppath[1024] = {0};
    
     if(!pname || !(*pname))
     {
         if(fmode & FILE_OPEN_WRITE)
             return STDOUT_FILENO;
         else
             return STDIN_FILENO;
     }
    
    sprintf(ppath, "%s%s", PIPE_HEAD, pname);
    
    if (access(ppath, F_OK) != 0)
        return INVALID_FILE;
    
    if(fmode & FILE_OPEN_WRITE)
        flag = O_WRONLY;
    else
        flag = O_RDONLY;
    
    if(fmode & FILE_OPEN_OVERLAP)
        flag |= O_NONBLOCK;

    rt = open(ppath, flag);
    
    return (rt < 0)? INVALID_FILE : (res_file_t)rt;
}

wait_t _pipe_wait(const tchar_t* pname, int ms)
{
    char ppath[1024] = {0};
    
    if(!pname || !(*pname))
    {
        return WAIT_RET;
    }

    sprintf(ppath, "%s%s", PIPE_HEAD, pname);
 
    while (ms && access(ppath, F_OK) != 0)
    {
        ms--;
        usleep(1000);
    }
    return (ms)? WAIT_RET : WAIT_TMO;
}

bool_t _pipe_write(res_file_t pipe, void* buf, dword_t len, async_t* pb)
{
    return _file_write(pipe, buf, len, pb);
}

bool_t _pipe_flush(res_file_t pipe)
{
    return _file_flush(pipe);
}

bool_t _pipe_read(res_file_t pipe, void* buf, dword_t size, async_t* pb)
{
    return _file_read(pipe, buf, size, pb);
}

#endif //XDK_SUPPORT_PIPE