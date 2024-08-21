/// @file      message/msglib.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "msglib.h"

#ifndef MAX_MAIL_INFO_COUNT
#define MAX_MAIL_INFO_COUNT 16 ///< 登録できる最大スレッド数
#endif

namespace
{
/// @brief メール情報
struct MailInfo
{
  osThreadId threadId; ///< スレッドID
  osMailQId mailId;    ///< メールID
};
/// メール情報実態
MailInfo s_mails[MAX_MAIL_INFO_COUNT] = {};
/// @brief 指定したスレッドIDと一致するメール情報取得
/// @param [in] threadId スレッドID
/// @retval 0以外　メール情報のポインタ
/// @retval 0 見つからない
MailInfo *find(osThreadId threadId)
{
  for (uint32_t i = 0; i < MAX_MAIL_INFO_COUNT; ++i)
  {
    MailInfo *info = &s_mails[i];
    if (info->threadId == threadId)
    {
      return info;
    }
  }
  return 0;
}
} // namespace

osStatus msg::registerThread(uint32_t msgCount) noexcept
{
  osThreadId threadId = osThreadGetId();
  if (threadId == 0)
  {
    return osErrorOS;
  }
  if (find(threadId) != 0)
  {
    return osErrorValue;
  }
  MailInfo *info = find(0);
  if (info == 0)
  {
    return osErrorNoMemory;
  }
  info->threadId = threadId;
  osMailQDef(mail, msgCount, Message);
  info->mailId = osMailCreate(osMailQ(mail), threadId);
  return info->mailId ? osOK : osErrorResource;
}

osStatus msg::send(osThreadId threadId, ID type) noexcept
{
  return send(threadId, type, 0, 0);
}

osStatus msg::send(osThreadId threadId, ID type, void const *bytes, uint16_t size) noexcept
{
  MailInfo *info = find(threadId);
  if (info == 0)
  {
    return osErrorParameter;
  }
  if (sizeof(Message::bytes) <= size)
  {
    return osErrorValue;
  }
  Message *m = static_cast<Message *>(osMailAlloc(info->mailId, 0));
  if (m == 0)
  {
    return osEventTimeout;
  }
  m->type = type;
  m->size = size;
  if (0 < size && bytes)
  {
    memcpy(m->bytes, bytes, size);
  }
  return osMailPut(info->mailId, m);
}

msg::Result msg::recv(uint32_t millisec) noexcept
{
  osThreadId threadId = osThreadGetId();
  if (threadId == 0)
  {
    return Result(osErrorOS);
  }
  MailInfo *info = find(threadId);
  if (info == 0)
  {
    return Result(osErrorParameter);
  }
  osEvent res = osMailGet(info->mailId, millisec);
  if (res.status != osEventMail)
  {
    return Result(res.status);
  }
  Message *m = static_cast<Message *>(res.value.p);
  osStatus st = m ? osOK : osErrorValue;
  return Result(st, m, info->mailId);
}

msg::Result::Result(osStatus status, Message *msg, osMailQId mail) noexcept //
    : status_(status),                                                      //
      msg_(msg),                                                            //
      mail_(mail)                                                           //
{
}
msg::Result::Result(osStatus status) noexcept //
    : status_(status),                        //
      msg_(0),                                //
      mail_(0)                                //
{
}
msg::Result::~Result()
{
  reset();
}
msg::Result::Result(Result &&that) noexcept //
    : status_(that.status_),                //
      msg_(that.msg_),                      //
      mail_(that.mail_)                     //
{
  that.status_ = osOK;
  that.msg_ = 0;
  that.mail_ = 0;
}
msg::Result &msg::Result::operator=(Result &&that) noexcept
{
  if (this != &that)
  {
    reset();
    status_ = that.status_;
    msg_ = that.msg_;
    mail_ = that.mail_;
    that.status_ = osOK;
    that.msg_ = 0;
    that.mail_ = 0;
  }
  return *this;
}
void msg::Result::reset() noexcept
{
  if (msg_)
  {
    osMailFree(mail_, msg_);
    msg_ = 0;
  }
}
osStatus msg::Result::status() const noexcept
{
  return status_;
}
msg::Message const *msg::Result::msg() const noexcept
{
  return msg_;
}