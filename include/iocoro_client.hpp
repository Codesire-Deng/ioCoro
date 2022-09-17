#pragma once

#include "iocoro_syscall.hpp"

namespace ioCoro {

template<typename Service>
class Client : public SeviceModelBase
{
public:
  Client() { m_tasks.Push(Alloc<PollOperation>(m_reactor, m_tasks, m_timer_holders)); }

  template<typename... Args>
  void Submit(Args&&... args)
  {
    Socket tmp{ *this };

    m_tasks.Push(
      Alloc<BaseOperation>(Service::Active(tmp, forward<Args>(args)...)));
  }

  template<typename F>
  void Submit(F&& func)
  {
    m_tasks.Push(Alloc<PostOperation<std::decay_t<F>>>(forward<F>(func)));
  }

  void join()
  {
    rx_store(m_join, true);
    
    m_join.wait(true, rx);

    Ios::Stop();
  }
};

} // namespace ioCoro