// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <event_explicit.h>
#include <event_loop.h>

#include <network/tcp/server/tcp_listen.h>
#include <network/tcp/server/tcp_server.h>
#include <network/tcp/client/tcp_connect.h>
#include <network/tcp/client/tcp_client.h>
#include <network/tcp/client/tcp_client_ex.h>
#include <event_buffer.h>

using namespace Evpp;

char test_url[41][256] =
{
    {"http://mirrors.163.com/centos/7/isos/x86_64/CentOS-7-x86_64-NetInstall-2009.iso"},
    {"http://mirrors.163.com/kernel/testing/old/v2.2/sha256sums.asc"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-base-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-common-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-common-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-common-debuginfo-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-debugsource-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-fuse-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-fuse-debuginfo-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-immutable-object-cache-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-immutable-object-cache-debuginfo-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-mds-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-mds-debuginfo-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-mgr-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-mds-debuginfo-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-mgr-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-mgr-debuginfo-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-mon-debuginfo-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-osd-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-radosgw-debuginfo-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-15.2.11/el8/aarch64/ceph-resource-agents-15.2.11-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-base-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-base-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-common-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-debugsource-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-fuse-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-fuse-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-immutable-object-cache-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-mds-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-mgr-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-mon-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-radosgw-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/ceph-test-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/cephfs-mirror-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/libradosstriper1-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/librbd1-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/librgw2-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/rbd-nbd-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/rbd-mirror-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/librbd1-debuginfo-16.2.4-0.el8.aarch64.rpm"},
    {"http://mirrors.163.com/ceph/rpm-16.2.4/el8/aarch64/libcephfs2-debuginfo-16.2.4-0.el8.aarch64.rpm"},
};


int main(int argc, char* argv [])
{
  using namespace Evpp;

  while (1)
  {
    {
      EventLoop ev;
      ev.InitEventBasic();
      TcpServer server(&ev);
      server.CreaterListen("0.0.0.0:18881");
      server.CreaterListen("0.0.0.0:18882");
      server.CreaterListen("0.0.0.0:18883");
      server.CreaterListen("0.0.0.0:18884");
      server.CreaterListen("[::]:18881");
      server.CreaterListen("[::]:18882");
      server.CreaterListen("[::]:18883");
      server.CreaterListen("[::]:18884");
      server.CreaterServer(16);
      //server.DestroyServer();

      ///cli.DestroyConnect();

     /* std::thread t1([](TcpClientEx* c1) { Sleep(0); c1->DestroyConnect(); }, &cli);
      t1.detach();*/

      ev.ExecDispatch();

      /*
       *
       *            //TcpClientEx cli(&ev, 2);
      //cli.InitialConnect(6);
      //for (u96 i = 0; i < 50; ++i)
      //{
      //    cli.AddConnect("127.0.0.1", 18888);
      //}

      //cli.CreaterConnect();
       *
       */
    }
  }

  return 0;
}