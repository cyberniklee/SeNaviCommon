#ifndef _MESSAGE_EVENT_H_
#define _MESSAGE_EVENT_H_

#include "../Time/Time.h"
#include "Declare.h"
#include "MessageTraits.h"

#include <boost/type_traits/is_void.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

namespace NS_NaviCommon
{

  template< typename M >
  struct DefaultMessageCreator
  {
    boost::shared_ptr< M > operator()()
    {
      return boost::make_shared< M >();
    }
  };

  template< typename M >
  inline boost::shared_ptr< M > defaultMessageCreateFunction()
  {
    return DefaultMessageCreator< M >()();
  }

  /**
   * \brief Event type for subscriptions, const MessageEvent<M const>& can be used in your callback instead of const boost::shared_ptr<M const>&
   *
   * Useful if you need to retrieve meta-data about the message, such as the full connection header, or the publisher's node name
   */
  template< typename M >
  class MessageEvent
  {
  public:
    typedef typename boost::add_const< M >::type ConstMessage;
    typedef typename boost::remove_const< M >::type Message;
    typedef boost::shared_ptr< Message > MessagePtr;
    typedef boost::shared_ptr< ConstMessage > ConstMessagePtr;
    typedef boost::function< MessagePtr() > CreateFunction;

    MessageEvent()
        : nonconst_need_copy_(true)
    {
    }

    MessageEvent(const MessageEvent< Message >& rhs)
    {
      *this = rhs;
    }

    MessageEvent(const MessageEvent< ConstMessage >& rhs)
    {
      *this = rhs;
    }

    MessageEvent(const MessageEvent< Message >& rhs, bool nonconst_need_copy)
    {
      *this = rhs;
      nonconst_need_copy_ = nonconst_need_copy;
    }

    MessageEvent(const MessageEvent< ConstMessage >& rhs,
                 bool nonconst_need_copy)
    {
      *this = rhs;
      nonconst_need_copy_ = nonconst_need_copy;
    }

    MessageEvent(const MessageEvent< void const >& rhs,
                 const CreateFunction& create)
    {
      init(
          boost::const_pointer_cast< Message >(
              boost::static_pointer_cast< ConstMessage >(rhs.getMessage())),
          rhs.getConnectionHeaderPtr(), rhs.getReceiptTime(),
          rhs.nonConstWillCopy(), create);
    }

    /**
     * \todo Make this explicit in ROS 2.0.  Keep as auto-converting for now to maintain backwards compatibility in some places (message_filters)
     */
    MessageEvent(const ConstMessagePtr& message)
    {
      init(message, boost::shared_ptr< StringMap >(), Time::now(), true,
           DefaultMessageCreator< Message >());
    }

    MessageEvent(const ConstMessagePtr& message,
                 const boost::shared_ptr< StringMap >& connection_header,
                 Time receipt_time)
    {
      init(message, connection_header, receipt_time, true,
           DefaultMessageCreator< Message >());
    }

    MessageEvent(const ConstMessagePtr& message, Time receipt_time)
    {
      init(message, boost::shared_ptr< StringMap >(), receipt_time, true,
           DefaultMessageCreator< Message >());
    }

    MessageEvent(const ConstMessagePtr& message,
                 const boost::shared_ptr< StringMap >& connection_header,
                 Time receipt_time, bool nonconst_need_copy,
                 const CreateFunction& create)
    {
      init(message, connection_header, receipt_time, nonconst_need_copy,
           create);
    }

    void init(const ConstMessagePtr& message,
              const boost::shared_ptr< StringMap >& connection_header,
              Time receipt_time, bool nonconst_need_copy,
              const CreateFunction& create)
    {
      message_ = message;
      connection_header_ = connection_header;
      receipt_time_ = receipt_time;
      nonconst_need_copy_ = nonconst_need_copy;
      create_ = create;
    }

    void operator=(const MessageEvent< Message >& rhs)
    {
      init(boost::static_pointer_cast< Message >(rhs.getMessage()),
           rhs.getConnectionHeaderPtr(), rhs.getReceiptTime(),
           rhs.nonConstWillCopy(), rhs.getMessageFactory());
      message_copy_.reset();
    }

    void operator=(const MessageEvent< ConstMessage >& rhs)
    {
      init(
          boost::const_pointer_cast< Message >(
              boost::static_pointer_cast< ConstMessage >(rhs.getMessage())),
          rhs.getConnectionHeaderPtr(), rhs.getReceiptTime(),
          rhs.nonConstWillCopy(), rhs.getMessageFactory());
      message_copy_.reset();
    }

    /**
     * \brief Retrieve the message.  If M is const, this returns a reference to it.  If M is non const
     * and this event requires it, returns a copy.  Note that it caches this copy for later use, so it will
     * only every make the copy once
     */
    boost::shared_ptr< M > getMessage() const
    {
      return copyMessageIfNecessary< M >();
    }

    /**
     * \brief Retrieve a const version of the message
     */
    const boost::shared_ptr< ConstMessage >&
    getConstMessage() const
    {
      return message_;
    }
    /**
     * \brief Retrieve the connection header
     */
    StringMap&
    getConnectionHeader() const
    {
      return *connection_header_;
    }
    const boost::shared_ptr< StringMap >&
    getConnectionHeaderPtr() const
    {
      return connection_header_;
    }

    /**
     * \brief Returns the name of the node which published this message
     */
    const std::string&
    getPublisherName() const
    {
      return
          connection_header_ ? (*connection_header_)["callerid"] : s_unknown_publisher_string_;
    }

    /**
     * \brief Returns the time at which this message was received
     */
    Time getReceiptTime() const
    {
      return receipt_time_;
    }

    bool nonConstWillCopy() const
    {
      return nonconst_need_copy_;
    }
    bool getMessageWillCopy() const
    {
      return !boost::is_const< M >::value && nonconst_need_copy_;
    }

    bool operator<(const MessageEvent< M >& rhs)
    {
      if(message_ != rhs.message_)
      {
        return message_ < rhs.message_;
      }

      if(receipt_time_ != rhs.receipt_time_)
      {
        return receipt_time_ < rhs.receipt_time_;
      }

      return nonconst_need_copy_ < rhs.nonconst_need_copy_;
    }

    bool operator==(const MessageEvent< M >& rhs)
    {
      return message_ = rhs.message_ && receipt_time_ == rhs.receipt_time_ && nonconst_need_copy_ == rhs.nonconst_need_copy_;
    }

    bool operator!=(const MessageEvent< M >& rhs)
    {
      return !(*this == rhs);
    }

    const CreateFunction&
    getMessageFactory() const
    {
      return create_;
    }

  private:
    template< typename M2 >
    typename boost::disable_if< boost::is_void< M2 >, boost::shared_ptr< M > >::type copyMessageIfNecessary() const
    {
      if(boost::is_const< M >::value || !nonconst_need_copy_)
      {
        return boost::const_pointer_cast< Message >(message_);
      }

      if(message_copy_)
      {
        return message_copy_;
      }

      assert(create_);
      message_copy_ = create_();
      *message_copy_ = *message_;

      return message_copy_;
    }

    template< typename M2 >
    typename boost::enable_if< boost::is_void< M2 >, boost::shared_ptr< M > >::type copyMessageIfNecessary() const
    {
      return boost::const_pointer_cast< Message >(message_);
    }

    ConstMessagePtr message_;
    // Kind of ugly to make this mutable, but it means we can pass a const MessageEvent to a callback and not worry about other things being modified
    mutable MessagePtr message_copy_;
    boost::shared_ptr< StringMap > connection_header_;
    Time receipt_time_;
    bool nonconst_need_copy_;
    CreateFunction create_;

    static const std::string s_unknown_publisher_string_;
  };

  template< typename M >
  const std::string MessageEvent< M >::s_unknown_publisher_string_(
      "unknown_publisher");

}

#endif // _MESSAGE_EVENT_H
