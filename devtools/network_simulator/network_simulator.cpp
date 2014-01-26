/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: Network simulator. Emulates Zigbee network topology.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "mac_frame.h"


/*
 TODO  Debug for Big Endian + Security !!
*/ 

using namespace std;

static bool g_exit = false;
static int g_nNode = 0;
static string g_xgml;
static map<int, vector<int> > g_visibility_matrix;
static bool g_use_visibility_matrix = false;
static string g_pipeName = "/tmp/znode";


int load_xgml(string xgml_path, map<int, vector<int> > &visibility_matrix);
int route_packet(uint8_t *raw_packet, uint16_t *nNode);


static void usage(int argc, char **argv)
{
  (void)argc;
  cout << "Usage:" << endl;
  cout << "\t" << argv[0] << " <--nNode=N> [--pipeName=<template>] [--xgml=<path_to_file>]" << endl;
  cout << "Options:" << endl;
  cout << "\t nNodes - Number of Zigbee nodes" << endl;
  cout << "\t pipeName - Pipe name template. Default /tmp/znode " << endl;
  cout << "\t xgml - Path to node visibility matrix file. By default all nodes see each other" << endl;
  cout << "Sample:" << endl;
  cout << "\t" << argv[0] << " --nNode=2 --pipeName=/tmp/znode --xgml=2nodes.xgml" << endl;
}


static inline int stl_str2int(const string s)
{
  int ret = 0;
  stringstream ss(s);
  ss >> ret;
  return ret;
}


static int parse_cmd(int argc, char **argv)
{
  int ret = -1;
  int i;

  for(i = 1; i < argc; ++i)
  {
    string token = argv[i];

    if ( token.length() < 2
         || token[0] != '-'
         || token[1] != '-'
         || token.find_first_of('=') == string::npos )
    {
      cout << "skip unknown parameter: " << token << endl;
      continue;
    }
    else
    {
      cout << "token: " << token << endl;
      token = token.substr(2);
      int break_at = token.find_first_of('=');
      string name = token.substr(0, break_at);
      string value = token.substr(break_at + 1) ;

      /* remove double quotes if used */
      remove(value.begin(), value.begin(), '"');
      remove(value.rbegin(), value.rbegin(), '"');

      if ( name == "nNode" )
      {
        g_nNode = stl_str2int(value);
        ret = 0; /* mandatory param is set */
      }
      else if ( name == "Join" )
      {
        // skip: compatibility with ns-3
      }
      else if ( name == "xgml" || name == "XGML" )
      {
        g_xgml = value;
      }
      else if ( name == "pipeName" || name == "PipeName")
      {
        g_pipeName = value;
      }
      else
      {
        cout << "skip unknown parameter: " << token << endl;
        continue;
      }
    }
  }

  return ret;
}


static void dddd(uint8_t *buf, const string comment)
{
  cout << "dump of buf [" << (unsigned)buf[0] << "] " << comment << endl;
  for (int i = 0; i < buf[0]; )
  {
    static char digits[] = "0123456789abcdef";
    int j;
    char s[16*3+1];
    for (j = 0 ; j < 16*3 && i < buf[0] ; i++)
    {
      s[j++] = digits[((buf[i] >> 4) & 0xf)];
      s[j++] = digits[(buf[i] & 0xf)];
      s[j++] = ' ';
    }
    s[j] = 0;
    cout << s << endl;
  }
}


static inline string get_rpipe_name(const string pattern, int i)
{
  stringstream ss;
  ss << pattern << i << ".read";
  return ss.str();
}


static inline string get_wpipe_name(const string pattern, int i)
{
  stringstream ss;
  ss << pattern << i << ".write";
  return ss.str();
}


/* Return pipe descriptor or -1 on error */
static inline int open_rpipe(const string name)
{
  int ret = -1;

  do
  {
    ret = open(name.c_str(), O_RDONLY | O_NONBLOCK);
    if ( ret < 0 )
    {
      cout << "read pipe " <<  name.c_str() << " open failed " << strerror(errno) << endl;
    }
  }
  while ( errno == EINTR );

  return ret;
}


/* Return pipe descriptor or -1 on error */
static inline int open_wpipe(const string name)
{
  int ret = -1;

  do
  {
    ret = open(name.c_str(), O_WRONLY | O_NONBLOCK);
    if ( ret < 0 )
    {
      cout << "write pipe " <<  name.c_str() << " open failed " << strerror(errno) << endl;
    }
  }
  while ( errno == EINTR );

  return ret;
}


static int read_data_from_pipe(int rpipe, vector<uint8_t> &data)
{
  int ret = 0;
  unsigned bts_read = 0;
  char buf[256];

  do
  {
    ret = read(rpipe, buf, sizeof(buf));
    if ( ret > 0 )
    {
      data.insert(data.end(), buf, buf + ret);
      bts_read += ret;
    }
    else if ( ret < 0 )
    {
      if ( errno != EINTR && errno != EAGAIN)
      {
        cout << "Read from pipe error " << strerror(errno) << endl;
      }
    }
  }
  while ( ret > 0 || errno == EINTR );

  if ( bts_read )
  {
    ret = bts_read;
  }

  cout << "Read from pipe: rpipe " << rpipe << " ret " << ret << endl;
  return ret;
}


static int send_data_to_pipe(vector<int> &wpipe, int node, uint8_t *buf)
{
  int ret = -1;
  int bts_written = 0;

  if ( wpipe[node] == -1 )
  {
    wpipe[node] = open_wpipe(get_wpipe_name(g_pipeName, node));
  }

  if ( wpipe[node] != -1 )
  {
    do
    {
      ret = write(wpipe[node], buf + bts_written, buf[0] - bts_written);
      if ( ret > 0 )
      {
        bts_written += ret;
      }
      else if ( ret < 0 )
      {
        cout << "unable to write to pipe, drop packet, close pipe" << endl;
        close(wpipe[node]);
        wpipe[node] = -1;
      }
    }
    while ( bts_written < buf[0]
            && (ret >= 0 || errno == EINTR) );
  }

  if ( ret > 0 )
  {
    ret = bts_written;
  }

  cout << "Write to pipe: pipe " << node << " ret " << ret << endl;
  return ret;
}


static inline int is_path_exist(uint16_t from, uint16_t to)
{
  map<int, vector<int> >::iterator it = g_visibility_matrix.find(from);
  return ( it != g_visibility_matrix.end()
           && find(it->second.begin(), it->second.end(), to) != it->second.end() );
}

static int send_data_thought_the_network(vector<int> &wpipe, vector<uint8_t> &data, uint16_t from_node)
{
  int ret = 0;
  uint8_t buf[256];
  uint16_t nNode;
  int i;

  while ( !g_exit
          && data.size()
          && (int)data[0] <= (int)data.size() )
  {
    /* copy to the temp buffer */
    copy(data.begin(), data.begin() + data[0], buf);
    data.erase(data.begin(), data.begin() + data[0]);

    dddd(buf, (char *)"buf parse");

    ret = route_packet(buf, &nNode);
    if ( ret >= 0 )
    {
      if ( nNode == 0xffff )
      {
        /* send to all nodes */
        for (i = 0; i < g_nNode; ++i)
        {
          if ( i != from_node )
          {
            if ( !g_use_visibility_matrix
                 || is_path_exist(from_node, i) )
            {
              ret = send_data_to_pipe(wpipe, i, buf);
            }
            else
            {
              cout << "No route from " << from_node << " to " << i << " Skip" << endl;
            }
          }
        }
      }
      else if ( nNode < g_nNode )
      {
        if ( !g_use_visibility_matrix
             || is_path_exist(from_node, nNode) )
        {
          ret = send_data_to_pipe(wpipe, nNode, buf);
        }
        else
        {
          cout << "No route from " << from_node << " to " << i << " Drop packet" << endl;
        }
      }
      else
      {
        cout << "wrong node number " << nNode << " drop packet" << endl;
      }
    }
    else
    {
      cout << "drop packet " << endl;
    }
  }

  return 0;
}

static void simulate()
{
  int ret;
  vector<int> rpipe;
  vector<int> wpipe;
  vector<uint8_t> data;
  int i;

  /* resize vectors */
  rpipe.resize(g_nNode, -1);
  wpipe.resize(g_nNode, -1);

  /* open read pipes */
  for (i = 0; i < g_nNode; ++i)
  {
    rpipe[i] = open_rpipe(get_rpipe_name(g_pipeName, i));
    if ( rpipe[i] < 0 )
    {
      goto done;
    }
  }

  while ( !g_exit )
  {
    static int select_timeout = 1;
    struct timeval tv;
    fd_set readSet;
    int maxfd = *max_element(rpipe.begin(), rpipe.end());

    FD_ZERO(&readSet);
    for (i = 0; i < g_nNode; ++i)
    {
      FD_SET(rpipe[i], &readSet);
    }

    tv.tv_sec = select_timeout;
    tv.tv_usec = 0;

    ret = select(maxfd + 1, &readSet, NULL, NULL, &tv);
    if ( ret >= 0 )
    {
      for (i = 0; i < g_nNode; ++i)
      {
        if ( FD_ISSET(rpipe[i], &readSet) )
        {
          cout << "have smth from node " << i << endl;
          ret = read_data_from_pipe(rpipe[i], data);
          if ( ret > 0 )
          {
            /* route packet */
            send_data_thought_the_network(wpipe, data, i);
          }
          else if ( ret == 0 )
          {
            cout << "node " << i << " rpipe error, try to reopen " << endl;
            /* reopen pipe */
            close(rpipe[i]);
            rpipe[i] = open_rpipe(get_rpipe_name(g_pipeName, i));
            if ( rpipe[i] < 0 )
            {
              goto done;
            }
          }
        }
      }
    }
    else
    {
      cout << "select error " << strerror(errno) << endl;
      sleep(1);
    }
  }

  done:
  cout << "exiting" <<endl;
  return;
}

int main(int argc, char *argv[])
{
  int ret = 0;
  int i;

  sigignore(SIGPIPE);

  /* parse command line */
  if ( (ret = parse_cmd(argc, argv)) )
  {
    usage(argc, argv);
    goto done;
  }

  /* load xgml visibility matrix */
  if ( g_xgml.length() )
  {
    load_xgml(g_xgml, g_visibility_matrix);
    if ( g_visibility_matrix.size() )
    {
      g_use_visibility_matrix = true;
    }
  }

  /* create pipes */
  for (i = 0; i < g_nNode; i++)
  {
    if ( mkfifo(get_rpipe_name(g_pipeName, i).c_str(),  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
         && errno != EEXIST )
    {
      cout << "mkfifo failed. " << strerror(errno) << endl;
      goto done;
    }

    if ( mkfifo(get_wpipe_name(g_pipeName, i).c_str(),  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
         && errno != EEXIST )
    {
      cout << "mkfifo failed. " << strerror(errno) << endl;
      goto done;
    }
  }

  cout << "start simulate" << endl;
  simulate();

  done:
  return ret;
}

