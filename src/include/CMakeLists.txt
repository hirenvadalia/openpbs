include(CheckIncludeFile)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckCSourceCompiles)

string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} PBS_MACH)

CHECK_INCLUDE_FILE(sys/eventfd.h HAVE_SYS_EVENTFD_H)
CHECK_INCLUDE_FILE(sys/ioctl.h HAVE_SYS_IOCTL_H)
check_symbol_exists(h_errno "netdb.h" H_ERRNO_DECLARED)
set(CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h")
check_type_size(socklen_t HAVE_SOCKLEN_T)

check_c_source_compiles("
#include <sys/epoll.h>
int main()
{
  return ((epoll_create(100) == -1) ? -1 : 0);
}
" PBS_HAVE_EPOLL)

check_c_source_compiles("
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <sys/epoll.h>
int main()
{
  sigset_t allsigs;
  int n;
  int maxevents = 1;
  int timeout = 0;
  int epollfd;
  struct   epoll_event  events;
  sigemptyset(&allsigs);
  events.events = EPOLLIN;
  epollfd = epoll_create1(0);
  if (epollfd == -1) {
    perror(\"epoll_create1\");
    return (1);
  }
  n = epoll_pwait(epollfd, &events, maxevents, timeout, &allsigs);
  return (n);
}
" PBS_HAVE_EPOLL_PWAIT)

check_c_source_compiles("
#include <unistd.h>
#include <poll.h>
#include <signal.h>
int main()
{
  sigset_t allsigs;
  int n;
  int fd[2];
  struct timespec timeoutspec;
  struct   pollfd  pollfds[1];
  timeoutspec.tv_nsec = 1000;
  timeoutspec.tv_sec = 0;
  pipe(fd);
  pollfds[0].fd = fd[0];
  sigemptyset(&allsigs);
  n = ppoll(pollfds, 1, &timeoutspec, &allsigs);
  return (n);
}" PBS_HAVE_PPOLL)

configure_file(pbs_config.h.in pbs_config.h)
