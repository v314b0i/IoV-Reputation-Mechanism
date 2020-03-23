//
// Generated file, do not edit! Created by nedtool 5.6 from veins/modules/application/traci/reportDumpMsg.msg.
//

#ifndef __VEINS_REPORTDUMPMSG_M_H
#define __VEINS_REPORTDUMPMSG_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif

// dll export symbol
#ifndef VEINS_API
#  if defined(VEINS_EXPORT)
#    define VEINS_API  OPP_DLLEXPORT
#  elif defined(VEINS_IMPORT)
#    define VEINS_API  OPP_DLLIMPORT
#  else
#    define VEINS_API
#  endif
#endif

// cplusplus {{
#include "veins/base/utils/Coord.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/base/utils/SimpleAddress.h"
#include "string"
// }}


namespace veins {

/**
 * Class generated from <tt>veins/modules/application/traci/reportDumpMsg.msg:28</tt> by nedtool.
 * <pre>
 * packet reportDumpMsg extends BaseFrame1609_4
 * {
 *     LAddress::L2Type senderAddress = -1;
 *     LAddress::L2Type reporteeAddress = -1;
 *     string trueMsgs; //commaseperated list of messages from reportee found to be true by sender
 *     string falseMsgs;//commaseperated list of messages from reportee found to be false by sender
 * }
 * </pre>
 */
class VEINS_API reportDumpMsg : public ::veins::BaseFrame1609_4
{
  protected:
    LAddress::L2Type senderAddress;
    LAddress::L2Type reporteeAddress;
    ::omnetpp::opp_string trueMsgs;
    ::omnetpp::opp_string falseMsgs;

  private:
    void copy(const reportDumpMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const reportDumpMsg&);

  public:
    reportDumpMsg(const char *name=nullptr, short kind=0);
    reportDumpMsg(const reportDumpMsg& other);
    virtual ~reportDumpMsg();
    reportDumpMsg& operator=(const reportDumpMsg& other);
    virtual reportDumpMsg *dup() const override {return new reportDumpMsg(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual LAddress::L2Type& getSenderAddress();
    virtual const LAddress::L2Type& getSenderAddress() const {return const_cast<reportDumpMsg*>(this)->getSenderAddress();}
    virtual void setSenderAddress(const LAddress::L2Type& senderAddress);
    virtual LAddress::L2Type& getReporteeAddress();
    virtual const LAddress::L2Type& getReporteeAddress() const {return const_cast<reportDumpMsg*>(this)->getReporteeAddress();}
    virtual void setReporteeAddress(const LAddress::L2Type& reporteeAddress);
    virtual const char * getTrueMsgs() const;
    virtual void setTrueMsgs(const char * trueMsgs);
    virtual const char * getFalseMsgs() const;
    virtual void setFalseMsgs(const char * falseMsgs);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const reportDumpMsg& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, reportDumpMsg& obj) {obj.parsimUnpack(b);}

} // namespace veins

#endif // ifndef __VEINS_REPORTDUMPMSG_M_H

