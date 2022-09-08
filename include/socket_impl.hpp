#pragma once

#include "operation.hpp"
#include "operation_queue.hpp"
#include "mm_order.hpp"

#include <arpa/inet.h>
#include <atomic>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using std::atomic;
using std::mutex;

namespace ioCoro {

struct SocketImpl
{
  atomic<bool> m_hided{};

  int m_fd{};

  struct ReactOperation : Operation
  {
    MetaOperation* meta{};

    atomic<bool> m_to_do{};

    SocketImpl* m_next{};

    static void perform(Operation* inBase)
    {
      ReactOperation* p = static_cast<ReactOperation*>(inBase);

      bool flag = true;

      /**
       * An atomic lock indicates some unwelcome guys that i know some
       * interesting things coming but pls look for someone else, This is not
       * your business
       */
      if (!acq_compare_exchange_strong(p->m_to_do, flag, false))
        return;

      if ((*(p->meta))())
        return;
      
      Operation* finish = static_cast<Operation*>(p->meta);
      (*finish)();
    }

    ReactOperation()
      : Operation{ &perform }
    {
    }
  } Ops{};

#define Next Ops.m_next
#define Task Ops.meta

  /**
   * There is an interesting thing here that em... why not a flag like
   * ReactOperation::m_to_do, or more deeply to say a mutex, in the class??
   */
  struct MaintainOperation : Operation
  {
    Operation* seed{};

    static void perform(Operation* inBase)
    {
      MaintainOperation* p = static_cast<MaintainOperation*>(inBase);

      (*(p->seed))();
    }

    MaintainOperation()
      : Operation{ &perform }
      , seed{}
    {
    }
  } Op{};

#define Seed Op.seed

  struct SockInfo
  {
    sockaddr_in addr{};
    socklen_t size{ sizeof(addr) };
  } m_info{};

#define Addr m_info.addr
#define Size m_info.size
};

} // namespace ioCoro